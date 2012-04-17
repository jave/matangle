/*
 * Copyright (c) 2012 Joakim Verona
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
  SCM_MAGIC_SNARFER is defined during function snarfing, which only needs the SCM macros
  to be expanded to work, so we dont include unecessary headers during this phase.
  (for some reason some headers were troublesome during snarfing)
*/
#ifndef SCM_MAGIC_SNARFER
#include <initializer_list> 
#include "triangle_actor.h"
#include <cluttermm.h>

#include <mx/mx.h>

#include <pangomm/init.h>
#include <pangomm/context.h>
#include <pangomm/layout.h>

#include <iostream>
#include <string>
#include <map>


#include <clutter-box2d.h>
#endif
#include <libguile.h>

double angle=0;

Glib::RefPtr<Tutorial::Triangle> selected_triangle;

ClutterTimeline *looptimeline;
float trih = 100.0 * 1.73205080757;
float triw = 200.0;
  ClutterActor *boxgroup;


int tricolx(int col){
  return col * triw/2;
}

int trirowy(int row){
  return row * trih;
}
double tricolrowrot(int col, int row){
  if(row % 2 == 0 ){
    //even row
    if(col % 2 == 0) return 180.0; else return 0.0;
  }else{
    //odd row
    if(col % 2 == 0) return 0.0; else return 180.0;
  }
    
}


void animation_callback (ClutterAnimation *button,
                      gpointer  user_data) {
  printf("animation %s\n", user_data);
  SCM func_symbol;
  SCM func;
  func_symbol = scm_c_lookup("animation-callback");
  func = scm_variable_ref(func_symbol);
  scm_call_1 (func, scm_from_latin1_string ((gchar*)user_data));
  
}


void swapTriangles(Glib::RefPtr<Tutorial::Triangle> t1, Glib::RefPtr<Tutorial::Triangle> t2){
  float t1x, t1y, t2x, t2y;
  //  t1->get_position(t1x, t1y);
  t1x=tricolx(t1->col);
  t1y=trirowy(t1->row);
  //t2->get_position(t2x, t2y);
  t2x=tricolx(t2->col);
  t2y=trirowy(t2->row);
  int tmp;

  tmp=t2->col;
  t2->col=t1->col;
  t1->col=tmp;

  tmp=t2->row;
  t2->row=t1->row;
  t1->row=tmp;
    
  gfloat v1,v2,x,y,z,tx,ty;
  v1  = tricolrowrot(t1->col,t1->row);// + t1->get_rotation(Clutter::Z_AXIS, x,y,z) + 360;
  v2  = tricolrowrot(t2->col,t2->row);// + t2->get_rotation(Clutter::Z_AXIS, x,y,z) + 360;

  //stop selection animation
  clutter_actor_detach_animation(t1->gobj());
   //clutter_timeline_stop(looptimeline);
  t1->set_scale(1.0,1.0);
  //swap the tiles places animated
ClutterAnimation* a1;
ClutterAnimation*   a2;
a1=clutter_actor_animate (t1->gobj(), CLUTTER_EASE_IN_OUT_QUINT, 1 * 1000,
                         "x", t2x,
                         "y", t2y,
                         "rotation_angle_z", v1,
                         NULL);
a2=clutter_actor_animate (t2->gobj(), CLUTTER_EASE_IN_OUT_QUINT, 1 * 1000,
                         "x", t1x,
                         "y", t1y,
                         "rotation_angle_z", v2,
                            NULL);
  g_signal_connect (G_OBJECT (    a1), "completed",    G_CALLBACK (animation_callback), (gchar*)"moved");
  g_signal_connect (G_OBJECT (    a2), "completed",    G_CALLBACK (animation_callback), (gchar*)"moved");

//inform guile about the impending swap
  SCM func_symbol;
  SCM func;
  func_symbol = scm_c_lookup("swap-grid");
  func = scm_variable_ref(func_symbol);
  scm_call_4 (func,
                 scm_from_unsigned_integer(t1->col),
               scm_from_unsigned_integer(t1->row),
               scm_from_unsigned_integer(t2->col),
                 scm_from_unsigned_integer(t2->row));


}


bool on_triangle_button_press(Clutter::ButtonEvent *event, const Glib::RefPtr<Tutorial::Triangle>& triangle){
  printf("click\n");

  if(selected_triangle){
    swapTriangles(selected_triangle, triangle);
    selected_triangle.reset();
  }else{
    selected_triangle = triangle;
    clutter_timeline_start(looptimeline);
    clutter_actor_animate_with_timeline (triangle->gobj(),
                                         CLUTTER_EASE_IN_CUBIC,
                                         looptimeline,
                                         "scale_x", 1.15,
                                         "scale_y", 1.15,
                                         NULL);

  }
  
  return false;
}




Glib::RefPtr<Tutorial::Triangle> addTriangle(double x, double y, std::string title, Clutter::Color color, int mode){
  //modes: 0 static; 1 reactive, 2 box2d
 // Add our custom actor to the stage:
    Glib::RefPtr<Clutter::Stage> stage = Clutter::Stage::get_default();
  Glib::RefPtr<Tutorial::Triangle> actor = Tutorial::Triangle::create(color);
  actor->set_size(triw, trih);

  //printf("trw:%f trih:%f\n",triw, trih);
  float w, h;
  actor->get_size(w,h);
  actor->setcolrow(x,y);

  //anchor and rotation are problematic with box2d
  if(mode!=2){
  actor->set_anchor_point( w /2.0,
                             h * 0.5//h  * 2/3
                             );
  }
  actor->set_position(tricolx(x), trirowy(y));

  
  actor->set_rotation(Clutter::Z_AXIS,tricolrowrot(x,y),0.0,0.0,0.0);

  actor->title=title;


  //dont add to stage when box2D
  //stage->add_actor(actor);
  


  //stuff for when using box2d
  clutter_group_add (CLUTTER_GROUP (boxgroup), actor->gobj());

  // //refactor this, see triangleactor
  //also this seems very hard to get right
  printf("scale:%f\n",clutter_box2d_get_scale_factor(CLUTTER_BOX2D(boxgroup)));
  float zw=triw;
  float zh=zw * (trih/triw); //this formula is totaly bogus trial and terror
  ClutterVertex v[]=
     {{triw / 2.0f / zw, 0.0, 0.0},
      {triw/zw,    trih/zh, 0.0},
      {0.0,     trih/zh, 0.0}
  };

  if(mode==0){
    //static unmovable actor
    clutter_container_child_set (CLUTTER_CONTAINER (boxgroup), actor->gobj(),
                                 "manipulatable", FALSE,
                                 "mode", CLUTTER_BOX2D_NONE, NULL);
    actor->set_reactive (FALSE);
  }
  if(mode==1){
    //reacts to clutter events but not box2d
    clutter_container_child_set (CLUTTER_CONTAINER (boxgroup), actor->gobj(),
                                 "manipulatable", FALSE,
                                 "mode", CLUTTER_BOX2D_NONE, NULL);
    actor->set_reactive (TRUE);
    actor->signal_button_press_event().connect(sigc::bind(sigc::ptr_fun(on_triangle_button_press), actor));
  }

  if(mode==2){
    //box2d actor
    clutter_container_child_set (CLUTTER_CONTAINER (boxgroup), actor->gobj(),
                                 "manipulatable", TRUE,
                                 "mode", CLUTTER_BOX2D_DYNAMIC, NULL);
    actor->set_reactive (TRUE);
  }

  
  //youre suposed to be able to use "outline", v in the above call, but its much easier to set it
  //after creation

  clutter_box2d_child_set_outline( CLUTTER_BOX2D(boxgroup), actor->gobj(),  v, 3);
  clutter_box2d_child_set_density( CLUTTER_BOX2D(boxgroup), actor->gobj(), 500.0);
  clutter_box2d_child_set_restitution( CLUTTER_BOX2D(boxgroup), actor->gobj(), 0.90);
  //end stuff for when using box2d

  actor->show();
  
  return actor;
}

SCM_DEFINE (addtriangle, "addtriangle", 5, 0, 0,
            (SCM x,SCM y, SCM title, SCM color, SCM mode),
            "add a triangle")
{
  Glib::RefPtr<Tutorial::Triangle> triangle = addTriangle(scm_to_int(x),scm_to_int(y), scm_to_utf8_stringn(title, NULL),
                                                          Clutter::Color(scm_to_int(scm_list_ref(color,scm_from_unsigned_integer(0))),
                                                                         scm_to_int(scm_list_ref(color,scm_from_unsigned_integer(1))),
                                                                         scm_to_int(scm_list_ref(color,scm_from_unsigned_integer(2))),
                                                                         scm_to_int(scm_list_ref(color,scm_from_unsigned_integer(3)))),
                                                                         scm_to_int(mode));


  
  return scm_from_pointer(triangle->gobj(), NULL); //for now TODO shape1
}

SCM_DEFINE (deltriangle, "deltriangle", 1, 0, 0,
              (SCM triangle),
            "delete a triangle")
{
  //crashes
  //printf("deltriangle x:%d", clutter_actor_get_x((ClutterActor*) SCM_POINTER_VALUE(scm_pointer_address(triangle))));
    printf("deltriangle\n");
    //printf("deltriangle x:%x", triangle);
    //clutter_actor_destroy((ClutterActor*) SCM_POINTER_VALUE(scm_pointer_address(triangle)));
    printf("deltriangle x:%d\n", SCM_POINTER_VALUE(triangle));
    clutter_actor_destroy((ClutterActor*) SCM_POINTER_VALUE(triangle));
  return SCM_BOOL_F;
}

//for box
static void
add_static_box (ClutterActor *group,
                gint          x,
                gint          y,
                gint          width,
                gint          height)
{
  ClutterActor *box;
  box = clutter_rectangle_new ();
  clutter_actor_set_size (box, width, height);
  clutter_actor_set_position (box, x, y);
  clutter_group_add (CLUTTER_GROUP (group), box);


  clutter_container_child_set (CLUTTER_CONTAINER (group), box,
                               "mode", CLUTTER_BOX2D_STATIC,
                               NULL);

}

//for box
void
add_cage (ClutterActor *group,
          gboolean      roof)
{
  ClutterActor *stage;
  gint width, height;

  stage = clutter_stage_get_default ();
  width = clutter_actor_get_width (stage);
  height = clutter_actor_get_height (stage);
  printf("cage w:%d h:%d\n",width,height);
  if (0)//roof)
    {
      //roof
      add_static_box (group, -100, -100, width + 200, 100);
    }
  else
    {
      //floor?
      add_static_box (group, -100, -height*(3-1)-100, width + 200, 100);
    }
  //floor?
  add_static_box (group, -100, height, width + 200, 100);

  //left wall? very high
  add_static_box (group, -200, -(height*(5-1)) , 100, height * 5);
  //right wall? very high
  add_static_box (group, width, -(height*(5-1)) , 100, height * 5);
}


SCM_DEFINE (box2dinit, "box2dinit", 1, 0, 0, (SCM dummy), "set up box2d scene")
{
  ClutterActor *stage;

  stage = clutter_stage_get_default ();

  boxgroup = clutter_box2d_new ();
  //clutter_box2d_set_scale_factor (CLUTTER_BOX2D (boxgroup), 0.0000001f);
  clutter_group_add (CLUTTER_GROUP (stage), boxgroup);
  //scene->group = boxgroup;
  add_cage (boxgroup, FALSE);
  
  
}
ClutterActor* scenecombo ;
SCM_DEFINE (getscenename, "getscenename", 0, 0, 0,
              (),
            "selected scene name")
{
  return scm_from_latin1_string(mx_combo_box_get_active_text (MX_COMBO_BOX(scenecombo)));
}

SCM_DEFINE (addscenename, "addscenename", 1, 0, 0,
              (SCM scene),
            "add scene name")
{
  mx_combo_box_append_text (MX_COMBO_BOX(scenecombo), scm_to_utf8_stringn(scene, NULL));
  return SCM_UNSPECIFIED;
}

#ifndef SCM_MAGIC_SNARFER
void snarf_init(){
  printf(" snarf_init\n");
#include "matangle.x"
 printf(" snarf_init end\n");
}
#endif

void button_callback (MxButton *button,
                      gpointer  user_data) {
  printf("button %s\n", user_data);
  SCM func_symbol;
  SCM func;
  func_symbol = scm_c_lookup("button-callback");
  func = scm_variable_ref(func_symbol);
  scm_call_1 (func, scm_from_latin1_string ((gchar*)user_data));
  
}

//int inner_main(int argc, char** argv)
static void inner_main(void *closure, int argc, char** argv)
{
    /* module initializations would go here */
      snarf_init();
  /* initialize GLib's threading support */
  g_thread_init (NULL);

  /* initialize Clutter's threading support */
  clutter_threads_init ();

 
  Clutter::init(&argc, &argv);
  Pango::init();
  // Get the stage and set its size and color:
  Glib::RefPtr<Clutter::Stage> stage = Clutter::Stage::get_default();
  stage->set_size(1400, 800);
    // Show the stage:
  stage->show();

  stage->set_fullscreen(); //doesnt actually change size of stage?


  stage->set_color(Clutter::Color(0x00, 0x00, 0x10, 0xFF)); // black

    //load background image
  Glib::RefPtr<Clutter::Texture> bg = Clutter::Texture::create_from_file("bg.jpg");
  stage->add_actor(bg);

  //add buttons
  ClutterActor* exitbutton = mx_button_new_with_label("exit"); //mx_menu_new();
  clutter_container_add_actor(CLUTTER_CONTAINER (stage->gobj()),exitbutton);

  ClutterActor* newbutton = mx_button_new_with_label("new"); 
  clutter_container_add_actor(CLUTTER_CONTAINER (stage->gobj()),newbutton);
  clutter_actor_set_position(CLUTTER_ACTOR (newbutton), 0,30);

  scenecombo = mx_combo_box_new (); 
  clutter_container_add_actor(CLUTTER_CONTAINER (stage->gobj()), scenecombo);
  // mx_combo_box_append_text (MX_COMBO_BOX(scenecombo), "accordion-colors");
  // mx_combo_box_append_text (MX_COMBO_BOX(scenecombo), "accordion-numbers");
  // mx_combo_box_append_text (MX_COMBO_BOX(scenecombo), "hexagon");
  // mx_combo_box_append_text (MX_COMBO_BOX(scenecombo), "box2d");
  clutter_actor_set_position(CLUTTER_ACTOR (scenecombo), 0,60);

  
  g_signal_connect (G_OBJECT (    exitbutton), "clicked",    G_CALLBACK (button_callback), (gchar*)"exit");
  g_signal_connect (G_OBJECT (    newbutton), "clicked",    G_CALLBACK (button_callback), (gchar*)"new");
      


  
  //init loop timeline
  looptimeline = clutter_timeline_new (500);
  clutter_timeline_set_loop (looptimeline, TRUE);
  clutter_timeline_set_auto_reverse (looptimeline, TRUE);
  

  
  //init box
  box2dinit(NULL);


  printf("starting guile server\n");

  scm_c_primitive_load("init.scm");
  printf("starting clutter\n");  


    /* acquire the main lock */
  clutter_threads_enter ();

  // Start the main loop, so we can respond to events:
  Clutter::main();

 /* release the main lock */
  clutter_threads_leave ();
  
  //return 0;
}


int
main (int argc, char **argv)
{

  scm_boot_guile (argc, argv, inner_main, 0);
  return 0; /* never reached */
}
