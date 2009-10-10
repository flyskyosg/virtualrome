/*=========================================================================
Program:   Dropper
Language:  C++
Authors:   Bruno Fanini
=========================================================================*/

#include <osgDB/ReadFile>
#include <osg/CoordinateSystemNode>

#include <osg/Switch>
#include <osgText/Text>

#include <osgUtil/IntersectionVisitor>
#include <osgUtil/IntersectVisitor>
#include <osgUtil/LineSegmentIntersector>

#include <osgDB/WriteFile>

#include <iostream>

// Name of generated Node containing Labels
#define OUTPUT_FILE_NAME	"LabelNode.ive"


int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    //arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is an application to drop labels on a given OSG model.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-T <filename>","Load terrain file");
    arguments.getApplicationUsage()->addCommandLineOption("-L <filename>","Load ASCII LabelFile. Format is <'Label Name'> <x> <y>");
	arguments.getApplicationUsage()->addCommandLineOption("--height float","Labels drop height");
	arguments.getApplicationUsage()->addCommandLineOption("--size float","Size of generated labels. Note this is useless with -f option.");
	arguments.getApplicationUsage()->addCommandLineOption("--font <font file>","Label Font");
	arguments.getApplicationUsage()->addCommandLineOption("-c <float> <float> <float> <float>","Label Color");
	arguments.getApplicationUsage()->addCommandLineOption("-b <float> <float> <float> <float>","Label Border Color");
	arguments.getApplicationUsage()->addCommandLineOption("-f <int>","Labels size is fixed-screen");

    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }
    
    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

	// Load the Terrain model
	osg::ref_ptr<osg::Node> loadedModel;

	std::string TerrainFile;
	if (arguments.read("-T", TerrainFile)){
		loadedModel = osgDB::readNodeFile( TerrainFile );
		}
	
	if (!loadedModel){
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
		}

	osg::BoundingBox bb;
	bb.expandBy( loadedModel.get()->getBound() );
	double Hmax,Hmin;
	Hmax = bb.corner(7).z();
	Hmin = bb.corner(0).z();
	std::cout << arguments.getApplicationName() <<": Model Z Extents: " << Hmax <<", "<< Hmin << std::endl;

	// Parse DeltaH
	float DeltaH = 0.0f;
	if (arguments.read("--height", DeltaH)){
		std::cout << arguments.getApplicationName() <<": Delta Height set = " << DeltaH << std::endl;
		}

	// Parse label size
	float LabelSize = 1.0f;
	if (arguments.read("--size", LabelSize)){
		std::cout << arguments.getApplicationName() <<": Label size set = " << LabelSize << std::endl;
		}

	// Parse label font file
	std::string LabelFont;
	if (arguments.read("--font", LabelFont)){
		std::cout << arguments.getApplicationName() <<": Label font set = " << LabelFont << std::endl;
		}

	// Parse label color
	osg::Vec4f LabelColor;
	float lr=1.0f,lg=1.0f,lb=1.0f,la=1.0f;
	if (arguments.read("-c", lr,lg,lb,la)){
		std::cout << arguments.getApplicationName() <<": Label color set." << std::endl;
		}
	LabelColor.set(lr,lg,lb, la);

	// Parse label border color
	osg::Vec4f LabelBorder;
	bool labelborderset = false;
	float bdr=0.0f,bdg=0.0f,bdb=0.0f,bda=1.0f;
	if (arguments.read("-b", bdr,bdg,bdb,bda)){
		std::cout << arguments.getApplicationName() <<": Label border color set." << std::endl;
		labelborderset = true;
		LabelBorder.set(bdr,bdg,bdb, bda);
		}

	// Parse fixed-screen size
	int FixedScreenSize = -1;
	if (arguments.read("-f", FixedScreenSize)){
		std::cout << arguments.getApplicationName() <<": Label fixed size on screen = " << FixedScreenSize << std::endl;
		}

	// Parse Label File
	std::string LabelFile;

	osg::ref_ptr<osg::Geode> LabelNode = new osg::Geode();
	unsigned int NumIntersections = 0;

	while (arguments.read("-L", LabelFile)){
		std::ifstream *in;
		in = new std::ifstream;
		in->open( LabelFile.c_str() );

		osgDB::FieldReader *FR;
		FR = new osgDB::FieldReader;

		FR->attach( in );

		// Label Node
		osg::ref_ptr<osg::Group> Lnode;
		Lnode = new osg::Group;

		// Parser
		while (!FR->eof()){
			osgDB::Field F;
			if ( FR->readField( F ) ){
				if (F.isQuotedString()){
					F.getWithinQuotes();
					std::string name = F.getStr();
					
					FR->readField( F );
					
					if ( F.isFloat() ){
						double x,y;

						F.getFloat(x);
						FR->readField( F );
						
						F.getFloat(y);
						std::cout << arguments.getApplicationName() <<": " << name << " is placed in ( " << x <<" , "<< y <<" )"<< std::endl;

						// Drop it
						osg::Vec3d start,end;
						start = osg::Vec3d(x,y,Hmax);
						end   = osg::Vec3d(x,y,Hmin);
						
						osg::ref_ptr<osgUtil::LineSegmentIntersector> L = new osgUtil::LineSegmentIntersector(start, end);
						osgUtil::IntersectionVisitor IV( L.get() );
						
						loadedModel.get()->accept( IV );

						if ( L->containsIntersections() ){
							NumIntersections++;
							osgUtil::LineSegmentIntersector::Intersections& intersections = L->getIntersections();
							
							// we are interested only in the first intersection
							osgUtil::LineSegmentIntersector::Intersections::iterator itr = intersections.begin();
							const osgUtil::LineSegmentIntersector::Intersection& it = *itr;

							osg::Vec3d P;
							P = it.localIntersectionPoint;
							P += osg::Vec3d(0.0,0.0,DeltaH);

							// Create and Add Label
							osg::ref_ptr<osgText::Text> Label = new osgText::Text;
							
							osg::ref_ptr<osgText::Font> LFont = osgText::readFontFile( LabelFont );
							Label->setFont( LFont.get() );

							Label->setAutoRotateToScreen(true);
							Label->setAlignment(osgText::Text::CENTER_BOTTOM);
							Label->setPosition( P );
							Label->setText( name );
							Label->setCharacterSize( LabelSize );
							
							Label->setColor( LabelColor );
							if (labelborderset){
								Label->setBackdropType( osgText::Text::OUTLINE );
								Label->setBackdropOffset( 0.1f );
								Label->setBackdropColor( LabelBorder );
								}

							if (FixedScreenSize > 0){
								Label->setCharacterSizeMode(osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
								Label->setFontResolution( FixedScreenSize, FixedScreenSize );
								}
							LabelNode.get()->addDrawable( Label.get() );
							}

						else std::cout << arguments.getApplicationName() <<": MISS! No intersection in " << x <<","<< y << std::endl;
						}

					}
				}
			//else std::cout << arguments.getApplicationName() <<": No field" << std::endl;
			}
		
		// close the stream and free
		in->close();
		delete( in );
		delete( FR );
		}

	// Write out the node.
	if (NumIntersections>0){
		osgDB::writeNodeFile(*LabelNode.get(),std::string(OUTPUT_FILE_NAME));
		//osgDB::Registry::instance()->writeNode( *LabelNode.get(), std::string("LabelNode.ive") );
		std::string n = OUTPUT_FILE_NAME;
		std::cout << arguments.getApplicationName() <<": Label Node " << n <<" has been generated."<< std::endl;
		}
	else std::cout << arguments.getApplicationName() <<": No Intersections, No party." << std::endl;

	return 0;
}
