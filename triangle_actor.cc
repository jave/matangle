//  Triangular actor for Matangle
//  written by Joakim Verona <joakim@verona.se>
// 
//  	Copyright (C) 2012 Joakim Verona
//  
//  This file is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public
//  License as published by the Free Software Foundation; either
//  version 3 of the License, or (at your option) any later version.
//  
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//  
//  You should have received a copy of the GNU General Public
//  License along with this file; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#include "triangle_actor.h"
#include <cogl/cogl.h>
#include <cogl-pango/cogl-pango.h>


#include <pangomm/init.h>
#include <pangomm/context.h>
#include <pangomm/layout.h>

namespace Tutorial
{

  Glib::RefPtr<Triangle> Triangle::create()
  {
    return Glib::RefPtr<Triangle>(new Triangle());
  }

  Glib::RefPtr<Triangle> Triangle::create(const Clutter::Color& color)
  {
    return Glib::RefPtr<Triangle>(new Triangle(color));
  }

  Triangle::Triangle()
    :
    color_ (0xFF, 0xFF, 0xFF, 0xFF)
  {


  }

  Triangle::Triangle(const Clutter::Color& color)
    :
    color_ (color)
  {}

  Triangle::~Triangle()
  {}



  
  
  void Triangle::do_triangle_paint(const CoglColor* color, bool paint)
  {
    const Clutter::Geometry geom = get_geometry();

    cogl_push_matrix();
    cogl_set_source_color(color);

    float outer_triangle[6];


    // Paint a triangle.  The parent paint call will have translated us into
    // position so paint from 0, 0.
    outer_triangle[0] = geom.get_width() / 2;
    outer_triangle[1] = 0.0;

    outer_triangle[2] = 0.0;
    outer_triangle[3] = geom.get_height();

    outer_triangle[4] = geom.get_width();
    outer_triangle[5] = outer_triangle[3];

    cogl_path_polygon(outer_triangle, G_N_ELEMENTS(outer_triangle) / 2);
    cogl_path_fill();


    if(paint){
      CoglColor coglcolor;
      cogl_color_set_from_4ub(&coglcolor,
                              255,0,0,255);

      cogl_set_source_color(&coglcolor);
      cogl_path_polygon(outer_triangle, G_N_ELEMENTS(outer_triangle) / 2);
      cogl_path_stroke();

      gfloat a,x,y,z,tx,ty,aa;


      //now place text
      int tw,th;
      Glib::RefPtr<Pango::Layout>  layoutx =  create_pango_layout(title);
      layoutx->set_alignment(Pango::ALIGN_CENTER);
      Pango::FontDescription font_desc("sans bold 40");
      layoutx->set_font_description(font_desc);
      layoutx->get_pixel_size(tw,th);

      //(note to self: this is way to complex. insted use group actor, triagle and text inside, and dont rotate text at all)
      //text rotates differently than the actor so we need the actor angle soon
      a  = get_rotation(Clutter::Z_AXIS, x,y,z);
      aa = sin(a /(2 * M_PI*4)); //the rotation might become more than a single rotation
      //text center (might) depend on rotation

      //2:  sqrt(1 + tan(pi / 6)^2) / 2
      //1:  0.57735026919

      
      tx = (geom.get_width() /2.0);//-(tw/2); //+ (aa/90-1)*(tw/2);
      ty = 0.57735026919*geom.get_width();//(geom.get_height()  /2.0);//-th/2;//*(aa);

      if(1){
      cogl_translate(tx,ty,0); //1st translate to text center

      cogl_rotate(-a,0,0,1); //then negate rotation around this point
      //printf("%f %f %f %d %d\n",a,tx,ty,tw,th);

      }
      //now render text, 0 0 is now in the middle of the triangle
      cogl_color_set_from_4ub(&coglcolor,
                              255,255,255,255);
      cogl_pango_render_layout(layoutx->gobj(),  -(tw/2),  -(th/2), &coglcolor, 0);

      //debug shape
      // cogl_color_set_from_4ub(&coglcolor,
      //                         0,255,0,255);
      // cogl_path_ellipse(0,0,1,1);
      // cogl_path_stroke();
    }
    cogl_pop_matrix();
  }

  void Triangle::on_paint()
  {
    CoglColor coglcolor;

    // Paint the triangle with the actor's color:
    cogl_color_set_from_4ub(&coglcolor,
                            color_.get_red(),
                            color_.get_green(),
                            color_.get_blue(),
                            color_.get_alpha()
                            );


  
    do_triangle_paint(&coglcolor, true);
  }

  void Triangle::on_pick(const Clutter::Color& color){
    // Paint the triangle with the pick color, offscreen.
    // This is used by Clutter to detect the actor under the cursor 
    // by identifying the unique color under the cursor.
    CoglColor coglcolor;
    cogl_color_set_from_4ub(&coglcolor,
                            color.get_red(),
                            color.get_green(),
                            color.get_blue(),
                            color.get_alpha());
    do_triangle_paint(&coglcolor, false);

  }
  /**
   * Tricoo::Triangle::get_color:
   *
   * @returns the color of the triangle.
   */
  Clutter::Color Triangle::get_color() const
  {
    return color_;
  }

  /**
   * Tricoo::Triangle::set_color:
   * @color: a Clutter::Color
   *
   * Sets the color of the triangle.
   */
  void Triangle::set_color(const Clutter::Color& color)
  {
    color_ = color;
    set_opacity(color_.get_alpha());

    if(is_visible())
      queue_redraw();
  }

  void Triangle::setcolrow(int col, int row){
    this->col=col;
    this->row=row;
  }
  
}
