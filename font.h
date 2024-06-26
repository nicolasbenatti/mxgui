/***************************************************************************
 *   Copyright (C) 2010, 2011 by Terraneo Federico                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#ifndef FONT_H
#define	FONT_H

#include "color.h"
#include "point.h"
#include "iterator_direction.h"
#include <algorithm>
#include <functional>
#include "../miosix-kernel/miosix/util/unicode.h"

namespace mxgui {

/**
 * \ingroup pub_iface
 * A Font that can be used to draw text. Fonts are immutable except they can be
 * assigned with operator=. Users are not expected to create instances of the Font
 * class directly. Rather, the default use case is to use the pre-instantiated
 * fonts in misc_inst.h, like droid11()
 */
class Font
{
public:
    /**
     * Creates a fixed width font.
     * \param blocks list of unicode blocks included in the font, in (base, size) pairs
     * \param height the height of the glyphs
     * \param width the width of the glyphs (fixed width font)
     * \param dataSize can be 8,16 or 32, it is the size of one element of data
     * \param antialiased true if font is antialiased
     * \param data pinter to the font data. This must point to a static array
     * so that no memory leak problems occur
     */
    constexpr Font(unsigned char numBlocks, const unsigned int *blocks, unsigned char height,
        unsigned char width, unsigned char dataSize, bool antialiased,
		const void *data, unsigned short numGlyphs): height(height), width(width),
        dataSize(dataSize), antialiased(antialiased), numGlyphs(numGlyphs),
		numBlocks(numBlocks), blocks(blocks), widths(0), offset(0), data(data) {}
	
    /**
     * Creates a variable width font.
     * \param blocks list of unicode blocks included in the font, (base, size) pairs
     * \param height the height of the glyphs
     * \param dataSize can be 8,16 or 32, it is the size of one element of data
     * \param antialiased true if font is antialiased
     * \param widths pointer to a table that contains the widths of each glyph.
     * This must point to a static array so that no memory leak problems occur
     * \param offset pointer to a table that contains where in data each glyph
     * begins (data[offset[c]])
     * This must point to a static array so that no memory leak problems occur
     * \param data pinter to the font data. This must point to a static array
     * so that no memory leak problems occur
     */
    constexpr Font(unsigned char numBlocks, const unsigned int *blocks, unsigned char height,
	    unsigned char dataSize, bool antialiased, const unsigned char *widths,
	    const unsigned short *offset, const void *data, unsigned short numGlyphs): height(height),
		width(0), dataSize(dataSize), antialiased(antialiased), numGlyphs(numGlyphs),
	    numBlocks(numBlocks), blocks(blocks), widths(widths), offset(offset), data(data) {}

    /**
     * Draw a string on a surface.
     * \param surface an object that provides pixel iterators.
     * \param colors colors for drawing antialiased text
     * \param p point of the upper left corner where the string will be drawn.
     * \param s string to write
     */
    template<typename T>
    void draw(T& surface, Color colors[4], Point p, const char *s) const;

    /**
     * Draw part of a string on a surface
     * \param surface an object that provides pixel iterators.
     * \param colors colors for drawing antialiased text
     * \param p point of the upper left corner where the string will be drawn.
     * Negative coordinates are allowed, as long as the clipped view has
     * positive or zero coordinates
     * \param a Upper left corner of clipping rectangle
     * \param b Lower right corner of clipping rectangle
     * \param s string to draw
     */
    template<typename T>
    void clippedDraw(T& surface, Color colors[4],
        Point p, Point a, Point b, const char *s) const;

    /**
     * Given a string, determine the length in pixels required to draw it.
     * If the string contains characters not between getStartChar and getEndChar
     * their width is supposed to be the length of the character returned by
     * getStartChar.
     * \param s a null terminated string
     * \return the length in pixels
     */
    short int calculateLength(const char *s) const;

    /**
     * \return true if the Font is fixed width
     */
    bool isFixedWidth() const { return width!=0; }

    /**
     * \return true if the Font is antialiased
     */
    bool isAntialiased() const { return antialiased; }

	/**
	   \return true if the codepoint is included in the Font
	*/
	bool isInRange(char32_t c) const;

	/**
	 * Translate a real codepoint in a virtual, 0-based index
	 * to access Font data tables
	 * \param codepoint the character codepoint
	 */
	unsigned int getVirtualCodepoint(char32_t codepoint) const;
	
    /**
     * \return the Font's height
     */
    unsigned char getHeight() const { return height; }

    /**
     * \return the Font's width. Use this member function only if the Font is
     * fixed width, otherwise see getWidths()
     */
    unsigned char getWidth() const { return width; }
    
    /**
     * \return number of glyphs
    */
    int getNumGlyphs() const { return numGlyphs; }

    /**
     * \return the size in bits of the data's data type.
     * Can be 8,16,32. For example if it is 16, data can be cast from
     * void* to unsigned short*
     */
    unsigned char getDataSize() const { return dataSize; }
	
    /**
     * \return the widths of the characters, only if it is a variable width
     * Font. If you want to know the width of character c when
     * using font f, use:
     * \code if(f.isInRange(c))
     * result=f.getWidths()[f.getVirtualCodepoint(c)] \endcode
     */
    const unsigned char *getWidths() const { return widths; }

    /**
     * \return a table with the offset within data where a character starts
     * For example, character c in font f starts at:
     * \code if(f.isInRange(c))
     * result=f.getOffset()[f.getVirtualCodepoint(c)] \endcode
     */
    const unsigned short *getOffset() const { return offset; }

    /**
     * \return a pointer to the font data, that can be used to draw a font.
     * The real datatype depends on getDataType()
     */
    const void *getData() const { return data; }

    /**
     * Generate 4 grayscale levels from foreground and background colors
     * for use with antialiased text drawing
     * \param out 
     * \param fgcolor
     * \param bgcolor
     */
    static void generatePalette(Color out[4], Color fgcolor, Color bgcolor);

    //Uses default copy constructor and operator=. The pointers can be shared
    //without problems since there is no member function to modify them
    //nor to return a non-const pointer to them
private:
	class GlyphDrawer
	{
	public:
		/**
		 * Draw a pixel column of a non-antialiased glyph
		 * \param it pixel iterator to the current on-screen position
		 * \param colors foreground & background colors
		 * \param col column of glyph pixels
		 */
		template<typename T, typename U>
		static inline void drawGlyphPixel(typename T::pixel_iterator& it, Color colors[2], U& col)
		{
			if(col & 0x1) *it=colors[0];
			else *it=colors[1];
			col>>=1;
		}
		
		/**
		 * Compute the amount of vertical pixels to skip
		 * when drawing the glyph. To be used ONLY in the case of
		 * clipped drawing
		 */
		static inline const short computeySkip(Point a, Point b)
		{
			return a.y()-b.y();
		}
	};
		
	class GlyphDrawerAA
	{
	public:
		/**
		 * Draw a pixel column of an antialiased glyph
		 * \param it pixel iterator to the current on-screen position
		 * \param colors palette for antialiased drawing
		 * \param col column of glyph pixels
		 */
		template<typename T, typename U>
		static inline void drawGlyphPixel(typename T::pixel_iterator& it, Color colors[4], U& col)
		{
			*it=colors[col & 0x3];
			col>>=2;
		}

		/**
		 * Compute the amount of vertical pixels to skip
		 * when drawing the glyph. To be used ONLY in the case of
		 * clipped drawing
		 */
		static inline const short computeySkip(Point a, Point b)
		{
			return (a.y()-b.y())*2;
		}
	};
	
	class FixedWidthGlyphLookup
	{
	public:
		/**
		 * Look up a fixed width glyph by its codepoint.
		 * \param virualCodepoint the character virtual codepoint
		 * \param col the glyph pixel column
		 */
		template<typename U>
		static inline U lookupGlyph(const Font *ref, unsigned int virtualCodepoint, unsigned char col)
		{
			const U *fontData=reinterpret_cast<const U *>(ref->getData());
			return fontData[(virtualCodepoint*ref->width)+col];
		}
	};

	class VariableWidthGlyphLookup
	{
	public:
		/**
		 * Look up variable width glyph data.
		 * \param virtualCodepoint the character virtual codepoint
		 * \param col the glyph pixel column 
		 */
		template<typename U>
		static inline U lookupGlyph(const Font *ref, unsigned int virtualCodepoint, unsigned char col)
		{
			const U *fontData=reinterpret_cast<const U *>(ref->getData());
			return fontData[ref->offset[virtualCodepoint]+col];
		}
	};

	class FixedWidth
	{
	public:
		static inline unsigned char getWidth(const Font *ref, unsigned int virtualCodepoint)
		{
			return ref->width;
		}
	};

	class VariableWidth
	{
	public:
		static inline unsigned char getWidth(const Font *ref, unsigned int virtualCodepoint)
		{
			return ref->widths[virtualCodepoint];
		}
	};
  
    /*
     * This is one nifty use of C++ templates. For size optimization purposes,
     * the elements in the font tables can either be 8, 16 or 32 bytes. This
     * however require 3 specialized algorithms for supporting them that only
     * differ in the cast from void* to unsigned char*, unsigned short* or
     * unsigned int*. Putting switch in the inner loop would be out of question
     * for speed reasons, and that triplication has to be done for each of the
     * six font drawing algorithms that deal with fixed width, variable width
     * and antialiased fonts, and their clipped versions.
     * Instead of writing each of these algorithms three times, it is written
     * only once using the U template parameter to specialize the code.
     * Thanks to the W, L and D template parameters, it's possible to furtherly
     * specialize the algorithm, minimizing duplication and writing just 2 
     * functions: one for standard drawing and one for clipped. In the end, 
     * on code size it does nothing worse since the alternative would have been 
     * to write 18 functions by hand, but on code compactness it allows to 
     * write only two functions and let template instantiation do the boring job.
     */

	/**
	 * Base algorithm for rendering a non-clipped font.
     * \param first pixel iterator to begin of drawing window
     * \param x start x coord
     * \param xEnd end x coord
     * \param colors background/foregound color pair
     * \param s string to write
	 */
	template<typename T,typename U, typename W, typename L, typename D>
	void drawingEngine(typename T::pixel_iterator first,
			short x, short xEnd, Color colors[], const char *s) const;

	/** 
     * Base algorithm for rendering a clipped font.
     * \param surface surface object providing pixel iterators
     * \param p point of the upper left corner where the string will be drawn
     * \param a upper left corner of non empty intersection
     * \param b lower right corner of non empty intersection
     * \param colors palette for antialiased drawing
     * \param s string to write
	 */
	template<typename T,typename U, typename W, typename L, typename D>
	void drawingEngineClipped(T& surface, Point p, Point a, Point b,
            Color colors[], const char *s) const;

    unsigned char height;
    unsigned char width;// set to zero if variable width font
    unsigned char dataSize;
    bool antialiased;
	unsigned short numGlyphs;
	unsigned char numBlocks;
	const unsigned int *blocks; // Codepoint ranges of the font
    const unsigned char *widths;// set to zero (NULL) if fixed width
    const unsigned short *offset;// set to zero (NULL) if fixed width
    const void *data;
};

template<typename T>
void Font::draw(T& surface, Color colors[4], Point p, const char *s) const
{
    //If no Y space to draw font, stop
    if(p.y()+height>surface.getHeight()) return;
    //If no X space to draw font, draw it until the screen margin reached
    typename T::pixel_iterator it;
    it=surface.begin(p,Point(surface.getWidth()-1,p.y()+height-1),DR);
    // For code size minimization not all the combinations of 8,16,32,64 bit
    // fixed, variable width and antialiased fonts are supported, but only these
    //  8 bit : none (too small for large displays)
    // 16 bit : fixedWidth, variableWidth
    // 32 bit : fixedWidth, variableWidth, variableWidthAntialiased
    // 64 bit : variableWidthAntialiased
	Color fgBgColors[2]={colors[3],colors[0]};
    switch(dataSize)
    {
        case 16:
            if(isAntialiased()) return;
            if(isFixedWidth())
                drawingEngine<T,unsigned short,FixedWidth,
						FixedWidthGlyphLookup,GlyphDrawer>(it,
						p.x(),surface.getWidth(),fgBgColors,s);
            else drawingEngine<T,unsigned short,VariableWidth,
						VariableWidthGlyphLookup,GlyphDrawer>(it,
						p.x(),surface.getWidth(),fgBgColors,s);
            break;
        case 32:
            if(isAntialiased())
            {
                if(isFixedWidth()) return;
                drawingEngine<T,unsigned int,VariableWidth,
						VariableWidthGlyphLookup,GlyphDrawerAA>(it,
						p.x(),surface.getWidth(),colors,s);
            } else {
                if(isFixedWidth())
                    drawingEngine<T,unsigned int,FixedWidth,
						FixedWidthGlyphLookup,GlyphDrawer>(it,p.x(),
						surface.getWidth(),fgBgColors,s);
                else drawingEngine<T,unsigned int,VariableWidth,
						VariableWidthGlyphLookup,GlyphDrawer>(it,
						p.x(),surface.getWidth(),fgBgColors,s);
            }
            break;
        case 64:
            if(isAntialiased()==false || isFixedWidth()) return;
            drawingEngine<T,unsigned long long,VariableWidth,
						VariableWidthGlyphLookup,GlyphDrawerAA>(it,
						p.x(),surface.getWidth(),colors,s);
            break;
    }
    it.invalidate(); //May not fill the requested window
}

template<typename T>
void Font::clippedDraw(T& surface, Color colors[4],
        Point p, Point a, Point b, const char *s) const
{
    using namespace std;
    //Find rectangle which is the non-empty intersection of the image rectangle
    //with the clip rectangle
    short ya=max(p.y(),a.y());
    short yb=min<short>(p.y()+this->getHeight()-1,b.y());
    if(ya>yb) return; //Empty intersection

    if(p.x()>b.x()) return; //Empty intersection
    short xa=max(p.x(),a.x());

    // For code size minimization not all the combinations of 8,16,32,64 bit
    // fixed, variable width and antialiased fonts are supported, but only these
    //  8 bit : none (too small for large displays)
    // 16 bit : fixedWidth, variableWidth
    // 32 bit : fixedWidth, variableWidth, variableWidthAntialiased
    // 64 bit : variableWidthAntialiased
	Color fgBgColors[2]={colors[3],colors[0]};
    switch(dataSize)
    {
        case 16:
            if(isAntialiased()) return;
            if(isFixedWidth())
				drawingEngineClipped<T,unsigned short,FixedWidth,
					   FixedWidthGlyphLookup,GlyphDrawer>(surface,p,
					   Point(xa,ya),Point(b.x(),yb),fgBgColors,s);
            else drawingEngineClipped<T,unsigned short,VariableWidth,
					   VariableWidthGlyphLookup,GlyphDrawer>(surface,p,
					   Point(xa,ya),Point(b.x(),yb),fgBgColors,s);
            break;
        case 32:
            if(isAntialiased())
            {
                if(isFixedWidth()) return;
                drawingEngineClipped<T,unsigned int,VariableWidth,
					   VariableWidthGlyphLookup,GlyphDrawerAA>(surface,p,
					   Point(xa,ya),Point(b.x(),yb),colors,s);
            } else {
                if(isFixedWidth())
                    drawingEngineClipped<T,unsigned int,FixedWidth,
					   FixedWidthGlyphLookup,GlyphDrawer>(surface,p,
					   Point(xa,ya),Point(b.x(),yb),fgBgColors,s);
                else drawingEngineClipped<T,unsigned int,VariableWidth,
					   VariableWidthGlyphLookup,GlyphDrawer>(surface,p,
					   Point(xa,ya),Point(b.x(),yb),fgBgColors,s);
            }
            break;
        case 64:
            if(isAntialiased()==false || isFixedWidth()) return;
            drawingEngineClipped<T,unsigned long long,VariableWidth,
					   VariableWidthGlyphLookup,GlyphDrawerAA>(surface,p,
					   Point(xa,ya),Point(b.x(),yb),colors,s);
            break;
    }
}

template<typename T,typename U, typename W, typename L, typename D>
void Font::drawingEngine(typename T::pixel_iterator first,
			short x, short xEnd, Color colors[], const char *s) const
{
    for(;;)
    {
        char32_t c = miosix::Unicode::nextUtf8(s);
        if(c=='\0') break;
		unsigned int vc=getVirtualCodepoint(c);
        for(unsigned int i=0;i<W::getWidth(this,vc);i++)
        {
            if(x++==xEnd) return;
            U row=L::template lookupGlyph<U>(this,vc,i);
            for(int j=0;j<height;j++)
            {
				D::template drawGlyphPixel<T,U>(first,colors,row);
                first++;
            }
        }
    }
}

template<typename T,typename U, typename W, typename L, typename D>
void Font::drawingEngineClipped(T& surface, Point p, Point a, Point b,
            Color colors[], const char *s) const
{
    //Walk the string till the first at least partially visible char
    int partial=0;
    short x=p.x();
    while(x<a.x())
    {
        char32_t c = miosix::Unicode::nextUtf8(s);
        if(c=='\0') return; //String ends before draw area begins
        unsigned int vc=getVirtualCodepoint(c);
        if(x+W::getWidth(this,vc)>a.x())
        {
            //The current char is partially visible
            partial=a.x()-x;
            break; //Don't go past this char, since it has to be drawn
        }
        x+=W::getWidth(this,vc);
    }
    x=a.x();

    typename T::pixel_iterator it=surface.begin(a,b,DR);

    const short ySkipped=D::computeySkip(a,p);
    const short yHeight=b.y()-a.y()+1;

    //Draw the first partially visible char, if it exists
    if(partial>0)
    {
        char32_t c = miosix::Unicode::nextUtf8(s);
        unsigned int vc=getVirtualCodepoint(c);
        for(unsigned int i=partial;i<W::getWidth(this,vc);i++)
        {
            if(x>b.x()) return;
            x++;
            U row=L::template lookupGlyph<U>(this,vc,i);
            row>>=ySkipped;
            for(int j=0;j<yHeight;j++)
            {
				D::template drawGlyphPixel<T,U>(it,colors,row);
            }
        }
    }

    //Draw the rest of the string
    for(;;)
    {
        char32_t c = miosix::Unicode::nextUtf8(s);
		unsigned int vc=getVirtualCodepoint(c);
        for(unsigned int i=0;i<widths[vc];i++)
        {
            if(x>b.x()) return;
            x++;
            U row=L::template lookupGlyph<U>(this,vc,i);
            row>>=ySkipped;
            for(int j=0;j<yHeight;j++)
            {
                D::template drawGlyphPixel<T,U>(it,colors,row);
            }
        }
    }
    it.invalidate(); //May not fill the requested window
}

} //namespace mxgui

#endif //FONT_H
