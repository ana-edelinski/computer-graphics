#version 330 core

layout(location = 0) in vec2 inPos;	//ulazni atribut koji dolazi iz vbo - tu se cuvaju podaci na grafickoj kartici, vec2 jer 2d (pozicije)
layout(location = 1) in vec4 inCol;	//drugi ulazni atribut - boja iz vbo, vec4 jer rgba
out vec4 chCol;	//ozlazni atribut vertex sejdera koji cu proslediti fragment sejderu
uniform vec2 uPos;	//uniform promenljiva za pomeranje kvadrata

void main()
{
	gl_Position = vec4(inPos + uPos, 0.0, 1.0);	//pomeranje vertiksa, ulaz + pomeraj, 0.0 jer je 2d (to je z koordinata), 1.0 homogena koordinata
	chCol = inCol;	//prenos boje (izlazni atribut chCol dobija vrednost ulazne boje inCol)
}