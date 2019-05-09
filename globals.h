#pragma once

/******* RTC *****/
String daysOfTheWeek[7] = { "Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado" };
String monthsNames[12] = { "Enero", "Febrero", "Marzo", "Abril", "Mayo",  "Junio", "Julio","Agosto","Septiembre","Octubre","Noviembre","Diciembre" };


byte BLOCK_1x8[8] = {
	0b00010000,
	0b00010000,
	0b00010000,
	0b00010000,
	0b00010000,
	0b00010000,
	0b00010000,
	0b00010000 };

byte BLOCK_2x8[8] = {
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000 };

byte BLOCK_3x8[8] = {
	0x1C,
	0x1C,
	0x1C,
	0x1C,
	0x1C,
	0x1C,
	0x1C,
	0x1C };

byte BLOCK_4x8[8] = {
	0x1E,
	0x1E,
	0x1E,
	0x1E,
	0x1E,
	0x1E,
	0x1E,
	0x1E };

byte BLOCK_5x8[8] = {
	0x1F,
	0x1F,
	0x1F,
	0x1F,
	0x1F,
	0x1F,
	0x1F,
	0x1F };

byte GRADO[8] = {
	0b00001100,
	0b00010010,
	0b00010010,
	0b00001100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000 };