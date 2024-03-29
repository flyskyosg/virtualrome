#ifndef __OSG4WEB_GIFREADER__
#define __OSG4WEB_GIFREADER__ 1



/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern  "C"
{
    #include <gif_lib.h>
}

#define ERR_NO_ERROR     0
#define ERR_OPEN         1
#define ERR_READ         2
#define ERR_MEM          3

#define MY_GIF_DEBUG 1



// GifImageStream class 
class GifImageStream : public osg::ImageStream, public OpenThreads::Thread
{
public:
	GifImageStream() : _length(0), 
		_dataNum(0), 
		_frameNum(0), 
        _done(false), 
		_currentLength(0), 
		_multiplier(1.0),
        osg::ImageStream() 
	{ 
		_status=PAUSED; 
	}

	virtual void play() 
	{ 
		if (!isRunning()) 
			start();
		_status=PLAYING; 
	}

	virtual void pause() { _status=PAUSED; }

	virtual void rewind() { setReferenceTime( 0.0 ); }

	virtual void quit( bool waitForThreadToExit=true ) 
	{
		_done = true;
		if ( waitForThreadToExit )
		{
			while( isRunning() ) 
				OpenThreads::Thread::YieldCurrentThread();
			osg::notify(osg::DEBUG_INFO)<<"GifImageStream thread quitted"<<std::endl;
		}
	}

	StreamStatus getStatus() { return _status; }
	virtual double getLength() const { return _length*0.01*_multiplier; }

	// Go to a specific position of stream
	virtual void setReferenceTime( double time ) 
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

        int i=1;
		int framePos = static_cast<int>(time*100.0/_multiplier);
		if ( framePos>=(int)_length )
			framePos = _length;

		std::vector<FrameData*>::iterator it;
		for ( it=_dataList.begin(); it!=_dataList.end(); it++,i++ )
		{
			framePos -= (*it)->delay;
			if ( framePos<0 )
				break;
		}
		_dataNum = i-1;
		_frameNum = (*it)->delay+framePos;
		setNewImage();
	}
    
	virtual double getReferenceTime() const { return _currentLength*0.01*_multiplier; }

	// Speed up, slow down or back to normal (1.0)
	virtual void setTimeMultiplier( double m ) 
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
		if ( m>0 )
			_multiplier = m;
	}
    
	virtual double getTimeMultiplier() const { return _multiplier; }
    
	// Not used in GIF animation
	virtual void setVolume(float) {}
	virtual float getVolume() const { return 0.0f; }

    virtual void run()
    {
        _dataIter = _dataList.begin();

        while ( !_done )
        {
            if ( _status==PLAYING && (*_dataIter) )
            {
                if ( _frameNum>=(*_dataIter)->delay )
                {
                    _frameNum = 0;
                    if ( _dataNum>=_dataList.size()-1 )
                    {
                        if ( getLoopingMode()==LOOPING )
                        {
                            _dataNum = 0;
                            _currentLength = 0;
                        }
                    }
                    else
                        _dataNum++;

                    setNewImage();
                }
                else
                {
                    _frameNum++;
                    _currentLength++;
                }

                OpenThreads::Thread::microSleep(static_cast<int>(10000.0f*_multiplier));
            }
            else
                OpenThreads::Thread::microSleep(150000L);
        }
    }

    void addToImageStream( int ss, int tt, int rr, int numComponents, int delayTime, unsigned char* imgData )
    {
        if ( isRunning() )
        {
            osg::notify(osg::WARN)<<"GifImageStream::addToImageStream: thread is running!"<<std::endl;
            return;
        }

        _s = ss;
        _t = tt;
        _r = rr;
        _internalFormat = numComponents;
        _dataType = GL_UNSIGNED_BYTE;

        _pixelFormat =
            numComponents == 1 ? GL_LUMINANCE :
            numComponents == 2 ? GL_LUMINANCE_ALPHA :
            numComponents == 3 ? GL_RGB :
            numComponents == 4 ? GL_RGBA : (GLenum)-1;

        if ( _dataList.empty() )
        {
            // Set image texture for the first time
            setImage(_s,_t,_r,_internalFormat,_pixelFormat,_dataType,
                imgData,osg::Image::NO_DELETE,1);
        }

        FrameData* newData = new FrameData;
        newData->delay = delayTime;
        newData->data = imgData;
        _dataList.push_back( newData );
        _length += delayTime;
    }

protected:
    typedef struct
    {
        unsigned int delay;
        unsigned char* data;
    } FrameData;

    void setNewImage()
    {
        _dataIter = _dataList.begin()+_dataNum;

        if ( *_dataIter )
        {
            unsigned char* image = (*_dataIter)->data;
            setImage(_s,_t,_r,_internalFormat,_pixelFormat,_dataType,
                image,osg::Image::NO_DELETE,1);
            dirty();
        }
    }

    virtual ~GifImageStream() 
    {
        if( isRunning() )
            quit( true );

        std::vector<FrameData*>::iterator it;
        for ( it=_dataList.begin(); it!=_dataList.end(); it++ )
        {
            delete (*it)->data;
            delete (*it);
        }
    }

    double _multiplier;
    unsigned int _currentLength;
    unsigned int _length;

    unsigned int _frameNum;
    unsigned int _dataNum;
    std::vector<FrameData*> _dataList;
    std::vector<FrameData*>::iterator _dataIter;

    int _s;
    int _t;
    int _r;
    int _internalFormat;
    unsigned int _pixelFormat;
    unsigned int _dataType;

    bool _done;
    OpenThreads::Mutex _mutex;
};




static int giferror = ERR_NO_ERROR;

int simage_gif_error(char * buffer, int buflen)
{
	switch (giferror)
	{
		case ERR_OPEN:
			strncpy(buffer, "GIF loader: Error opening file", buflen);
			break;
		case ERR_READ:
			strncpy(buffer, "GIF loader: Error reading file", buflen);
			break;
		case ERR_MEM:
			strncpy(buffer, "GIF loader: Out of memory error", buflen);
			break;
	}
	return giferror;
}


int simage_gif_identify(const char *, const unsigned char *header, int headerlen)
{
	static unsigned char gifcmp[] = {'G', 'I', 'F'};
	if (headerlen < 3) return 0;
	if (memcmp((const void*)header,
		(const void*)gifcmp, 3) == 0) return 1;
	return 0;
}


static void decode_row(GifFileType * giffile, unsigned char * buffer, unsigned char * rowdata, int x, int y, int len, int transparent)
{
	GifColorType * cmentry;
	ColorMapObject * colormap;
	int colormapsize;
	unsigned char col;
	unsigned char * ptr;
	y = giffile->SHeight - (y+1);
	ptr = buffer + (giffile->SWidth * y + x) * 4;

	colormap = (giffile->Image.ColorMap ? giffile->Image.ColorMap : giffile->SColorMap);
	colormapsize = colormap ? colormap->ColorCount : 255;

	while (len--)
	{
		col = *rowdata++;
                    
		if (col >= colormapsize) col = 0;
        
		if ( col == transparent )
		{
			// keep pixels of last image if transparent mode is on
			// this is necessary for GIF animating 
			ptr += 3;
		}
		else
		{
			cmentry = colormap ? &colormap->Colors[col] : NULL;
			if (cmentry)
			{
				*ptr++ = cmentry->Red;
				*ptr++ = cmentry->Green;
				*ptr++ = cmentry->Blue;
			}
			else
			{
				*ptr++ = col;
				*ptr++ = col;
				*ptr++ = col;
			}
		}
		*ptr++ = (col == transparent ? 0x00 : 0xff);
	}
}

int gif_read_stream(GifFileType *gfile, GifByteType *gdata, int glength)
{
	std::istream *stream = (std::istream*)gfile->UserData; //Get pointer to istream
	stream->read((char*)gdata,glength); //Read requested amount of data
	return stream->gcount();
}

unsigned char *simage_gif_load(std::istream& fin, int *width_ret, int *height_ret, int *numComponents_ret, GifImageStream** obj)
{
	int i, j, n, row, col, width, height, extcode;
	unsigned char * rowdata;
	unsigned char * buffer, * ptr;
	unsigned char bg;
	int transparent, delaytime;
	GifRecordType recordtype;
	GifByteType * extension;
	GifFileType * giffile;
	GifColorType * bgcol;

	int interlacedoffset[] = { 0, 4, 2, 1 };
	int interlacedjumps[] = { 8, 8, 4, 2 };

	giffile = DGifOpen(&fin,gif_read_stream);
	if (!giffile)
	{
		giferror = ERR_OPEN;
		return NULL;
	}

	transparent = -1;           
	delaytime = 8;    

	n = giffile->SHeight * giffile->SWidth;
	buffer = new unsigned char [n * 4];
	if (!buffer)
	{
		giferror = ERR_MEM;
		return NULL;
	}
	
	rowdata = new unsigned char [giffile->SWidth];
	if (!rowdata)
	{
		giferror = ERR_MEM;
		delete [] buffer;
		return NULL;
	}

	bg = giffile->SBackGroundColor;
	if (giffile->SColorMap && bg < giffile->SColorMap->ColorCount)
	{
		bgcol = &giffile->SColorMap->Colors[bg];
	}
	else bgcol = NULL;
	ptr = buffer;
	for (i = 0; i < n; i++)
	{
		if (bgcol)
		{
			*ptr++ = bgcol->Red;
			*ptr++ = bgcol->Green;
			*ptr++ = bgcol->Blue;
			*ptr++ = 0xff;
		}
		else
		{
			*ptr++ = 0x00;
			*ptr++ = 0x00;
			*ptr++ = 0x00;
			*ptr++ = 0xff;
		}
	}

	int gif_num=0;
	do
	{
		if (DGifGetRecordType(giffile, &recordtype) == GIF_ERROR)
		{
			giferror = ERR_READ;
			delete [] buffer;
			delete [] rowdata;
			return NULL;
		}
		switch (recordtype)
		{
			case IMAGE_DESC_RECORD_TYPE:
				
				gif_num++;
				if ( gif_num==2 )
				{
					*obj = new GifImageStream;
					(*obj)->addToImageStream( giffile->SWidth, giffile->SHeight, 1, 4, delaytime, buffer );
					unsigned char* destbuffer = new unsigned char [n * 4];
					buffer = (unsigned char*)memcpy( destbuffer, buffer, n*4 );
				}

				if (DGifGetImageDesc(giffile) == GIF_ERROR)
				{
					giferror = ERR_READ;
					delete [] buffer;
					delete [] rowdata;
					return NULL;
				}
				
				row = giffile->Image.Top;
				col = giffile->Image.Left;
				width = giffile->Image.Width;
                height = giffile->Image.Height;
                if (giffile->Image.Left + giffile->Image.Width > giffile->SWidth ||
                    giffile->Image.Top + giffile->Image.Height > giffile->SHeight)
                {
                   
                    giferror = ERR_READ;
                    delete [] buffer;
                    delete [] rowdata;
                    return NULL;
                }
                if (giffile->Image.Interlace)
                {
                    //fprintf(stderr,"interlace\n");
                    
                    for (i = 0; i < 4; i++)
                    {
                        for (j = row + interlacedoffset[i]; j < row + height;
                            j += interlacedjumps[i])
                        {
                            if (DGifGetLine(giffile, rowdata, width) == GIF_ERROR)
                            {
                                giferror = ERR_READ;
                                delete [] buffer;
                                delete [] rowdata;
                                return NULL;
                            }
                            else decode_row(giffile, buffer, rowdata, col, j, width, transparent);
                        }
                    }
                }
                else
                {
                    for (i = 0; i < height; i++, row++)
                    {
                        if (DGifGetLine(giffile, rowdata, width) == GIF_ERROR)
                        {
                            giferror = ERR_READ;
                            delete [] buffer;
                            delete [] rowdata;
                            return NULL;
                        }
                        else decode_row(giffile, buffer, rowdata, col, row, width, transparent);
                    }
                }

                // Record gif image stream 
                if ( *obj && obj )
                {
                    (*obj)->addToImageStream( giffile->SWidth, giffile->SHeight, 1, 4, delaytime, buffer );
                    unsigned char* destbuffer = new unsigned char [n * 4];
                    buffer = (unsigned char*)memcpy( destbuffer, buffer, n*4 );
                }


                break;
            case EXTENSION_RECORD_TYPE:
              
                if (DGifGetExtension(giffile, &extcode, &extension) == GIF_ERROR)
                {
                    giferror = ERR_READ;
                    delete [] buffer;
                    delete [] rowdata;
                    return NULL;
                }
           
                else if (extcode == 0xf9)
                {
                    if (extension[0] >= 4 && extension[1] & 0x1) transparent = extension[4];
                    else transparent = -1;

                    delaytime = (extension[3]<<8)+extension[2];    // minimum unit 1/100s, so 8 here means 8/100s 
                }
                while (extension != NULL)
                {
                    if (DGifGetExtensionNext(giffile, &extension) == GIF_ERROR)
                    {
                        giferror = ERR_READ;
                        delete [] buffer;
                        delete [] rowdata;
                        return NULL;
                    }
                }
                break;
            case TERMINATE_RECORD_TYPE:
                break;
            default:
                break;
        }
    }
    while (recordtype != TERMINATE_RECORD_TYPE);

    delete [] rowdata;
    *width_ret = giffile->SWidth;
    *height_ret = giffile->SHeight;
    *numComponents_ret = 4;
    DGifCloseFile(giffile);
    return buffer;
}


ReadResult readGIFStream(std::istream& fin)
{
	unsigned char *imageData = NULL;
	int width_ret;
	int height_ret;
	int numComponents_ret;

	GifImageStream* gifStream = NULL;
	imageData = simage_gif_load( fin,&width_ret,&height_ret,&numComponents_ret, &gifStream );

	switch (giferror)
	{
		case ERR_OPEN:
			return ReadResult("GIF loader: Error opening file");
		case ERR_READ:
			return ReadResult("GIF loader: Error reading file");
		case ERR_MEM:
			return ReadResult("GIF loader: Out of memory error");
	}

	// Use GifImageStream to display animate GIFs 
	if ( gifStream )
	{
		osg::notify(osg::DEBUG_INFO)<<"Using GifImageStream ..."<<std::endl;
		return gifStream;
	}

	if (imageData==NULL) 
		return ReadResult::FILE_NOT_HANDLED;

	int s = width_ret;
	int t = height_ret;
	int r = 1;

	int internalFormat = numComponents_ret;

	unsigned int pixelFormat =
		numComponents_ret == 1 ? GL_LUMINANCE :
		numComponents_ret == 2 ? GL_LUMINANCE_ALPHA :
		numComponents_ret == 3 ? GL_RGB :
		numComponents_ret == 4 ? GL_RGBA : (GLenum)-1;

	unsigned int dataType = GL_UNSIGNED_BYTE;

	osg::Image* pOsgImage = new osg::Image;

	pOsgImage->setImage(s,t,r, internalFormat, pixelFormat, dataType, imageData, osg::Image::USE_NEW_DELETE);

	return pOsgImage;
}


*/

#endif //__OSG4WEB_GIFREADER__