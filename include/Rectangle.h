#ifndef RECTANGLE_H
#define RECTANGLE_H

/**
 * Creates a rectangle struct.
 * 
 * References resources and tutorials provided by Professor Card through
 * the "CSC 481-581 HW 1-4.pdf" located beneath the "Homework 1" title in
 * the "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
 * Moodle page. These resources can be found in the
 * https://wiki.libsdl.org/SDL2/FrontPage website. Particularly based on
 * tutorials and guidance from the wiki for drawing entities.
 */
class Rectangle{
    public:
        int x,y,width,height;
        Rectangle(int x=0, int y=0, int width=0, int height=0);
};

#endif