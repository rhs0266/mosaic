#include <stdio.h>
#include <assert.h>
#include "EasyBMP.h"

#define NM 100005
#define FOR(i,n,m) for (int i=(n);i<=(m);i++)

#define RGB 0
#define GRAYSCALE 1

BMP pic, res;
int exist[NM];


// change size of BMP A to (W,H) and fit to (x,y)~(x+W-1,y+H-1) of target BMP
BMP warp(BMP A, int W, int H, int type = RGB) {
	int w = A.TellWidth(), h = A.TellHeight();
	BMP B; B.SetSize(W, H);

	double sourceRatio = (double) w / h;
	double destRatio = (double) W / H;

	FOR(i, 0, W - 1) {
		FOR(j, 0, H - 1) {
			RGBApixel rgba;
			
			int I = (double)i * w / W;
			int J = (double)j * h / H;
			rgba = A.GetPixel(I, J);

			if (type == GRAYSCALE) {
				double t = 0.2989 * rgba.Red + 0.5870 * rgba.Green + 0.1140 * rgba.Blue;
				rgba.Red = rgba.Green = rgba.Blue = t;
			}
			
			B.SetPixel(i, j, rgba);
		}
	}
	return B;
}

RGBApixel interpolate(RGBApixel A, RGBApixel B, double alpha) { // alpha*A + (1-alpha)*B
	RGBApixel C;
	C.Red = A.Red * alpha + B.Red * (1 - alpha);
	C.Blue = A.Blue * alpha + B.Blue * (1 - alpha);
	C.Green = A.Green * alpha + B.Green * (1 - alpha);
	return C;
}

RGBApixel transform(RGBApixel A, double to, double from) { // limit <= A' <= 1-limit
	double p = A.Red;
	if (from > 1e-6) p = p / from * to;
	else p = to;
	if (p > 254) p = 254;
	RGBApixel C;
	C.Red = C.Blue = C.Green = p;
	return C;	
}


int main() {

	int M = 0;
	int w = 90, h = 60; // size
	int columnNum = 200, rowNum = 200;
	double alpha = 0.4;

	int Num = 8000, N = 0;
	assert(N < NM);
	
	// read
	FOR(i, 1, Num) {
		char name[20];
		sprintf(name, "./BMP/IMG_%04d.bmp", i);
		if (pic.ReadFromFile(name))
			exist[++N] = i;
	}

	BMP T; T.ReadFromFile("./BMP/original.bmp");
	BMP original = warp(T, w*columnNum, h*rowNum, GRAYSCALE);

	// warp
	res.SetSize(w * columnNum, h * rowNum);
	FOR(i, 0, columnNum - 1) {
		FOR(j, 0, rowNum - 1) {
			M++; if (M == N + 1) M = 1;

			char name[20];
			int num = rand() % N + 1;
			sprintf(name, "./BMP/IMG_%04d.bmp", exist[num]);
			pic.ReadFromFile(name);
			BMP X = warp(pic, w, h, GRAYSCALE);

			/*
			// interpolate version (target * alpha + original * (1 - alpha))
			FOR(k, 0, w - 1) {
				FOR(l, 0, h - 1) {
					RGBApixel temp = X.GetPixel(k, l);
					RGBApixel temp2 = original.GetPixel(w*i + k, h*j + l);
					res.SetPixel(w * i + k, h * j + l, interpolate(temp, temp2, alpha));
				}
			}
			*/

			// transform version (color transform, average of target and original are same)
			double avgFrom = 0.0, avgTo = 0.0;
			FOR(k, 0, w - 1) {
				FOR(l, 0, h - 1) {
					avgTo += original.GetPixel(w*i + k, h*j + l).Red;
					avgFrom += X.GetPixel(k, l).Red;
				}
			}
			avgTo /= w*h; avgFrom /= w*h;

			FOR(k, 0, w - 1) {
				FOR(l, 0, h - 1) {
					RGBApixel temp = X.GetPixel(k, l);
					res.SetPixel(w * i + k, h * j + l, transform(temp, avgTo, avgFrom));
				}
			}

			if (j % 10 == 0) printf("%d ", j);
		}
		printf("End %d\n", i);
	}
	res.WriteToFile("./result.bmp");

	return 0;
}