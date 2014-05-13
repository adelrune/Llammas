/*
 * pitchbendArray.h
 *
 * Copyright 2014 Guillaume Riou, Aude Forcione-Lambert & Nicolas Hurtubise.
 *
 * This file is part of Llammas.
 *
 * Llammas is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Llammas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Llammas.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <avr/pgmspace.h>
// La grosse ligne incomprehensible est un macro utilisé par GCC pour mettre la variable dans
// la memoire flash du microcontroleur.
const float __attribute__((section(".progmem.data"))) PB_ARRAY[] = {

    -1.0, -0.984375, -0.96875, -0.953125, -0.9375, -0.921875, -0.90625, -0.890625, 
    -0.875, -0.859375, -0.84375, -0.828125, -0.8125, -0.796875, -0.78125, -0.765625,
    -0.75, -0.734375, -0.71875, -0.703125, -0.6875, -0.671875, -0.65625, -0.640625,
    -0.625, -0.609375, -0.59375, -0.578125, -0.5625, -0.546875, -0.53125, -0.515625,
    -0.5, -0.484375, -0.46875, -0.453125, -0.4375, -0.421875, -0.40625, -0.390625,
    -0.375, -0.359375, -0.34375, -0.328125, -0.3125, -0.296875, -0.28125, -0.265625,
    -0.25, -0.234375, -0.21875, -0.203125, -0.1875, -0.171875, -0.15625, -0.140625, 
    -0.125, -0.109375, -0.09375, -0.078125, -0.0625, -0.046875, -0.03125, -0.015625,
    0.0, 0.015625, 0.03125, 0.046875, 0.0625, 0.078125, 0.09375, 0.109375, 0.125, 
    0.140625, 0.15625, 0.171875, 0.1875, 0.203125, 0.21875, 0.234375, 0.25, 0.265625,
    0.28125, 0.296875, 0.3125, 0.328125, 0.34375, 0.359375, 0.375, 0.390625, 0.40625,
    0.421875, 0.4375, 0.453125, 0.46875, 0.484375, 0.5, 0.515625, 0.53125, 0.546875,
    0.5625, 0.578125, 0.59375, 0.609375, 0.625, 0.640625, 0.65625, 0.671875, 0.6875,
    0.703125, 0.71875, 0.734375, 0.75, 0.765625, 0.78125, 0.796875, 0.8125, 0.828125,
    0.84375, 0.859375, 0.875, 0.890625, 0.90625, 0.921875, 0.9375, 0.953125, 0.97275,
    1.0

};

