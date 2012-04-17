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

#ifndef CLUTTER_TUTORIAL_TRIANGLE_ACTOR_H
#define CLUTTER_TUTORIAL_TRIANGLE_ACTOR_H

#include <cluttermm.h>

namespace Tutorial
{

class Triangle : public Clutter::Actor
{
public:
  std::string title;
  int row;
  int col;
  void setcolrow(int row, int col);
  static Glib::RefPtr<Triangle> create();
  static Glib::RefPtr<Triangle> create(const Clutter::Color& color);

  virtual ~Triangle();

  void set_color(const Clutter::Color& color);
  Clutter::Color get_color() const;
  Glib::RefPtr<Pango::Context> context_;
  Glib::RefPtr<Pango::Layout> layout;  
protected:
  Triangle();
  explicit Triangle(const Clutter::Color& color);

  virtual void on_paint();
  virtual void on_pick(const Clutter::Color& color);
private:
  Clutter::Color color_;

  void do_triangle_paint(const CoglColor* color, bool paint);
};

} 

#endif
