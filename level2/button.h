/***************************************************************************
 *   Copyright (C) 2014 by Terraneo Federico                               *
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

#ifndef BUTTON_H
#define	BUTTON_H

#include 
#include <config/mxgui_settings.h>

#include "application.h"
#include "label.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui {

/**
 * A basic interactive Button.
 */
class Button : public Drawable
{
public:
    /**
     * Constructor
     * The object will be immediately enqueued for redraw
     * \param w window to which this object belongs
     * \param da area on screen occupied by this object
     * \param text text written in the Button
     */
    Button(Window *w, DrawArea da, const std::string& text="");
    
    /**
     * Constructor
     * The object will be immediately enqueued for redraw
     * \param w window to which this object belongs
     * \param p upper left point of the button
     * \param width width of the button
     * \param height height of the button
     * \param text text written in the button
     */
    Button(Window *w, Point p, short width, short height, const std::string& text="");
    
    
    /**
     * Change the text being displayed
     * \param text new text to be displayed
     */
    void setText(const std::string& text);
   
    /**
     * Set the function to be called when the button is pressed
     * \param callback pointer to the function to be called
     */
    void setCallback(void (*callback)(int));
   

    void buttonDown();

    
    void buttonUp();
 
    
    /**
     * \internal
     * Overridden this member function to draw the object.
     * \param dc drawing context used to draw the object
     */
    virtual void onDraw(DrawingContextProxy& dc);
    
private:
    Label *text; ///< Text of the button
    void (*callback)(int)=NULL; ///< Pointer to the function to be called when the button is pressed
    bool clicked=false; ///< True if the button is clicked
    Point innerPointTl; ///< Upper left point of the inner area of the button
    Point innerPointBr; ///< Lower right point of the inner area of the button
};

} //namesapce mxgui

#endif //MXGUI_LEVEL_2

#endif //BUTTON_H
