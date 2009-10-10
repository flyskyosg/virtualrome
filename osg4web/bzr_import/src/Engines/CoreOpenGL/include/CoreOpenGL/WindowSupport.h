#ifndef __OSG4WEB_WINDOWSUPPORT__
#define __OSG4WEB_WINDOWSUPPORT__ 1


namespace WindowSupport
{
	class WindowDimension
	{	
		public:	
			WindowDimension() : _x(0), _y(0), _w(1), _h(1) { }
			WindowDimension(int x, int y, int w, int h) : _x(x), _y(y), _w(w), _h(h) {}

			void set(int x, int y, int w, int h) { _x = x; _y = y; _w = w; _h = h; }
	
			void setX(int x) { _x = x; }
			void setY(int y) { _y = y; }
			void setWidth(int w) { _w = w; }
			void setHeight(int h) { _h = h; }

			int getX() { return _x; }
			int getY() { return _y; }
			int getWidth() { return _w; }
			int getHeight() { return _h; }

		private:
			int _x, _y, _w, _h;
		};
};



#endif //__OSG4WEB_WINDOWSUPPORT__