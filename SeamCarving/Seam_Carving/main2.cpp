#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>
#pragma warning (disable:4996)

#pragma comment(linker,"/subsystem:console")

// Include SDL lib
#include "Headers/SDL2-2.0.8/include/SDL.h"
#include "Headers/SDL2-2.0.8/include/SDL_image.h"
#include "Headers/SDL2-2.0.8/include/SDL_mixer.h"
#pragma comment(lib,"Headers\\SDL2-2.0.8\\lib\\x86\\SDL2.lib")
#pragma comment(lib,"Headers\\SDL2-2.0.8\\lib\\x86\\SDL2main.lib")
#pragma comment(lib,"Headers\\SDL2-2.0.8\\lib\\x86\\SDL2.lib")
#pragma comment(lib,"Headers\\SDL2-2.0.8\\lib\\x86\\SDL2_image.lib")
#pragma comment(lib,"Headers\\SDL2-2.0.8\\lib\\x86\\SDL2_mixer.lib")


SDL_Window* window;
int screen_width = 1700;
int screen_height = 1080;


void to_Grayscale(unsigned char* pixels, int h, int w, float** grayscale)
{
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			float magnitude = 0.0;
			int index = 3 * w * i + 3 * j;
			magnitude += pixels[index];
			magnitude += pixels[index + 1];
			magnitude += pixels[index + 2];
			grayscale[i][j] = magnitude / 3;
		}
	}
}

void copy_RGB(unsigned char* src, int h, int w, unsigned char** red, unsigned char** green, unsigned char** blue)
{
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			int index = 3 * w * i + 3 * j;
			red[i][j] = src[index];
			green[i][j] = src[index + 1];
			blue[i][j] = src[index + 2];
		}
	}
}

void to_Gradient(float** grayscale, int h, int w, float** gradient)
{
	float Gx[3][3] = { {-1,0,1}, {-2,0,2}, {-1,0,1} };
	float Gy[3][3] = { {-1,-2,-1}, {0,0,0}, {1,2,1} };

	for (int i = 0; i < h - 2; i++)
	{
		for (int j = 0; j < w - 2; j++)
		{
			float s1 = 0, s2 = 0;
			for (int k1 = 0; k1 < 3; k1++)
			{
				for (int k2 = 0; k2 < 3; k2++)
				{
					s1 += Gx[k1][k2] * grayscale[i+k1][j+k2];
					s2 += Gy[k1][k2] * grayscale[i+k1][j+k2];
				}
			}
			gradient[i][j] = sqrt(s1*s1 + s2 * s2);
		}
	}

	for (int i = 0; i < w; i++)
	{
		gradient[h - 1][i] = grayscale[h - 1][i];
		gradient[h - 2][i] = grayscale[h - 2][i];
	}
	for (int i = 0; i < h; i++)
	{
		gradient[i][w - 1] = grayscale[i][w - 1];
		gradient[i][w - 2] = grayscale[i][w - 2];
	}
}

void filter(int h, int w, float** gradient)
{
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			if (gradient[i][j] < 100) gradient[i][j] = 0.0;
		}
	}
}

void print_Grid(float **grid, int h, int w, int start_x, int start_y, SDL_Renderer* renderer)
{
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			SDL_Rect rect = { start_x + j,start_y + i,1,1 };
			SDL_SetRenderDrawColor(renderer, grid[i][j], grid[i][j], grid[i][j], 0);
			SDL_RenderFillRect(renderer, &rect);
		}
	}
}

float get_Least_Important_Column(float** gradient, int h, int w, int* pos)
{
	float** lowest = (float**)malloc(sizeof(float*)*h);
	for (int i = 0; i < h; i++)
		lowest[i] = (float*)malloc(sizeof(float)*w);

	int** track = (int**)malloc(sizeof(int*)*h);
	for (int i = 0; i < h; i++)
		track[i] = (int*)malloc(sizeof(int)*w);

	for (int i = 0; i < w; i++)
		lowest[0][i] = gradient[0][i];

	for (int i = 1; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			float tmp = lowest[i - 1][j];
			track[i][j] = j;
			if (j > 0 && lowest[i - 1][j - 1] < tmp)
			{
				tmp = lowest[i - 1][j - 1];
				track[i][j] = j - 1;
			}
			if (j < w - 1 && lowest[i - 1][j + 1] < tmp)
			{
				tmp = lowest[i - 1][j + 1];
				track[i][j] = j + 1;
			}
			lowest[i][j] = tmp + gradient[i][j];
		}
	}

	float min = lowest[h - 1][0];
	int position = 0;
	for (int i = 1; i < w; i++)
	{
		if (lowest[h - 1][i] < min)
		{
			min = lowest[h - 1][i];
			position = i;
		}
	}

	int k = h - 1;
	while (k >= 0)
	{
		pos[k] = position;
		position = track[k][position];
		k--;
	}

	for (int i = 0; i < h; i++)
		free(lowest[i]);
	free(lowest);

	for (int i = 0; i < h; i++)
		free(track[i]);
	free(track);
	return min;
}

void erase_Least_Important_Column(int* pos, float** grayscale, unsigned char** red, unsigned char** green, unsigned char** blue, int* h, int* w)
{
	for (int i = 0; i < *h; i++)
	{
		for (int j = pos[i]; j < *w - 1; j++)
		{
			grayscale[i][j] = grayscale[i][j + 1];
			red[i][j] = red[i][j + 1];
			green[i][j] = green[i][j + 1];
			blue[i][j] = blue[i][j + 1];
		}
	}
	(*w)--;
}

void read_Filenames(char** filenames, int* num_of_files)
{
	FILE* file = fopen("filelist.txt", "r");
	for (;;)
	{
		char* p = fgets(filenames[*num_of_files], 255, file);
		if (p == NULL) break;
		int length = strlen(filenames[*num_of_files]);
		if (filenames[*num_of_files][length - 1] == '\n') filenames[*num_of_files][length - 1] = '\0';
		(*num_of_files)++;
	}
}

float find_Average_Column(float** gradient, int img_h, int img_w)
{
	float sum = 0.0;
	for (int i = 0; i < img_h; i++)
	{
		for (int j = 0; j < img_w; j++)
			sum += gradient[i][j];
	}
	return sum / img_w;
}


int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Carving", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	// Initialize
	unsigned char** red = (unsigned char**)malloc(sizeof(unsigned char*)*screen_height);
	unsigned char** green = (unsigned char**)malloc(sizeof(unsigned char*)*screen_height);
	unsigned char** blue = (unsigned char**)malloc(sizeof(unsigned char*)*screen_height);
	for (int i = 0; i < screen_height; i++)
	{
		red[i] = (unsigned char*)malloc(sizeof(unsigned char)*screen_width);
		green[i] = (unsigned char*)malloc(sizeof(unsigned char)*screen_width);
		blue[i] = (unsigned char*)malloc(sizeof(unsigned char)*screen_width);
	}
	float** grayscale = (float**)malloc(sizeof(float*)* screen_height);
	for (int i = 0; i < screen_height; i++)
		grayscale[i] = (float*)malloc(sizeof(float)* screen_width);
	float** gradient = (float**)malloc(sizeof(float*)* screen_height);
	for (int i = 0; i < screen_height; i++)
		gradient[i] = (float*)malloc(sizeof(float)* screen_width);
	unsigned char* pixels = (unsigned char*)malloc(sizeof(unsigned char)*screen_height*screen_width * 3);

	int* pos = (int*)malloc(sizeof(int)*screen_height);

	char** filenames = (char**)malloc(sizeof(char*) * 100);
	for (int i = 0; i < 100; i++)
		filenames[i] = (char*)malloc(sizeof(char) * 255);
	int num_of_files = 0;
	// Read files
	read_Filenames(filenames, &num_of_files);

	int current = 0;
	SDL_Surface* image = IMG_Load(filenames[current]);
	SDL_Texture* image_t = SDL_CreateTextureFromSurface(renderer, image);

	int img_h = image->h;
	int img_w = image->pitch / 3;

	copy_RGB((unsigned char*)image->pixels, img_h, img_w, red, green, blue);
	to_Grayscale((unsigned char*)image->pixels, img_h, img_w, grayscale);
	to_Gradient(grayscale, img_h, img_w, gradient);
	filter(img_h, img_w, gradient);
	float avg = find_Average_Column(gradient, img_h, img_w);
	int last_change = SDL_GetTicks();
	int current_time = 0;
	int stage = 0;
	int next = 0;
	int stop = 0;

	SDL_RenderClear(renderer);
	for (;;)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				exit(0);
			}
			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_RETURN) next = 1;
			}
		}
		current_time = SDL_GetTicks();

		if (next == 1)
		{
			current = (current + 1) % num_of_files;
			image = IMG_Load(filenames[current]);
			image_t = SDL_CreateTextureFromSurface(renderer, image);
			img_h = image->h;
			img_w = image->pitch / 3;
			copy_RGB((unsigned char*)image->pixels, img_h, img_w, red, green, blue);
			to_Grayscale((unsigned char*)image->pixels, img_h, img_w, grayscale);
			to_Gradient(grayscale, img_h, img_w, gradient);
			filter(img_h, img_w, gradient);
			avg = find_Average_Column(gradient, img_h, img_w);
			SDL_RenderClear(renderer);
			last_change = SDL_GetTicks();
			stage = 0;
			next = 0;
			continue;
		}


		// Stage 0
		if (stage == 0)
		{
			SDL_Rect tmp = { (screen_width - img_w) / 2,(screen_height - img_h) / 2,img_w,img_h };
			SDL_RenderCopyEx(renderer, image_t, NULL, &tmp, 0, 0, SDL_FLIP_NONE);
			SDL_RenderPresent(renderer);
			stage++;
			continue;
		}

		// Stage 1
		if ((current_time - last_change > 2000) && (stage == 1))
		{
			to_Gradient(grayscale, img_h, img_w, gradient);
			filter(img_h, img_w, gradient);
			int index = 0;
			for (int i = 0; i < img_h; i++)
			{
				for (int j = 0; j < img_w; j++)
				{
					if (gradient[i][j] > 0)
					{
						pixels[index++] = 20;
						pixels[index++] = 20;
						pixels[index++] = 20;
						continue;
					}
					else if (i > 0 && gradient[i - 1][j] > 0)
					{
						pixels[index++] = 20;
						pixels[index++] = 20;
						pixels[index++] = 20;
						continue;
					}
					else if (j > 0 && gradient[i][j - 1] > 0)
					{
						pixels[index++] = 20;
						pixels[index++] = 20;
						pixels[index++] = 20;
						continue;
					}
					else if (i > 0 && j > 0 && gradient[i - 1][j - 1] > 0)
					{
						pixels[index++] = 20;
						pixels[index++] = 20;
						pixels[index++] = 20;
						continue;
					}
					pixels[index++] = blue[i][j];
					pixels[index++] = green[i][j];
					pixels[index++] = red[i][j];
				}
			}
			SDL_Surface* gray_image = SDL_CreateRGBSurfaceFrom(pixels, img_w, img_h, 24, img_w * 3, 0, 0, 0, 0);
			SDL_Texture* gray_image_texture = SDL_CreateTextureFromSurface(renderer, gray_image);
			SDL_Rect tmp = { (screen_width - img_w) / 2,(screen_height - img_h) / 2,img_w,img_h };
			SDL_RenderCopyEx(renderer, gray_image_texture, NULL, &tmp, 0, 0, SDL_FLIP_NONE);
			SDL_RenderPresent(renderer);
			SDL_DestroyTexture(gray_image_texture);
			SDL_FreeSurface(gray_image);
			stage++;
			continue;
		}
		
		// Stage 2
		if ((current_time - last_change > 4000) && (stage == 2))
		{
			if (stage == 3) continue;
			for (int i = 0; i < 5; i++)
			{
				to_Gradient(grayscale, img_h, img_w, gradient);
				filter(img_h, img_w, gradient);
				float lowest = get_Least_Important_Column(gradient, img_h, img_w, pos);
				if  (img_w < image->w*0.75 || lowest > avg /15)
				{
					stage++;
					break;
				}
				erase_Least_Important_Column(pos, grayscale, red, green, blue, &img_h, &img_w);
			}
			int index = 0;
			for (int i = 0; i < img_h; i++)
			{
				for (int j = 0; j < img_w; j++)
				{
					if (gradient[i][j] > 0)
					{
						pixels[index++] = 20;
						pixels[index++] = 20;
						pixels[index++] = 20;
						continue;
					}
					else if (i > 0 && gradient[i - 1][j] > 0)
					{
						pixels[index++] = 20;
						pixels[index++] = 20;
						pixels[index++] = 20;
						continue;
					}
					else if (j > 0 && gradient[i][j - 1] > 0)
					{
						pixels[index++] = 20;
						pixels[index++] = 20;
						pixels[index++] = 20;
						continue;
					}
					else if (i > 0 && j > 0 && gradient[i - 1][j - 1] > 0)
					{
						pixels[index++] = 20;
						pixels[index++] = 20;
						pixels[index++] = 20;
						continue;
					}
					pixels[index++] = blue[i][j];
					pixels[index++] = green[i][j];
					pixels[index++] = red[i][j];
				}
			}
			SDL_Surface* cropped_image = SDL_CreateRGBSurfaceFrom(pixels, img_w, img_h, 24, img_w * 3, 0, 0, 0, 0);
			SDL_Texture* cropped_image_texture = SDL_CreateTextureFromSurface(renderer, cropped_image);
			SDL_Rect tmp = { (screen_width - img_w) / 2,(screen_height - img_h) / 2,img_w,img_h };
			SDL_RenderClear(renderer);
			SDL_RenderCopyEx(renderer, cropped_image_texture, NULL, &tmp, 0, 0, SDL_FLIP_NONE);
			SDL_RenderPresent(renderer);
			SDL_DestroyTexture(cropped_image_texture);
			SDL_FreeSurface(cropped_image);
		}

		if (stage == 3)
		{
			int index = 0;
			for (int i = 0; i < img_h; i++)
			{
				for (int j = 0; j < img_w; j++)
				{
					pixels[index++] = blue[i][j];
					pixels[index++] = green[i][j];
					pixels[index++] = red[i][j];
				}
			}
			SDL_Surface* cropped_image = SDL_CreateRGBSurfaceFrom(pixels, img_w, img_h, 24, img_w * 3, 0, 0, 0, 0);
			SDL_Texture* cropped_image_texture = SDL_CreateTextureFromSurface(renderer, cropped_image);
			SDL_Rect tmp = { (screen_width - img_w) / 2,(screen_height - img_h) / 2,img_w,img_h };
			SDL_RenderClear(renderer);
			SDL_RenderCopyEx(renderer, cropped_image_texture, NULL, &tmp, 0, 0, SDL_FLIP_NONE);
			SDL_RenderPresent(renderer);
			SDL_DestroyTexture(cropped_image_texture);
			SDL_FreeSurface(cropped_image);
			stage = 4;
			stop = SDL_GetTicks();
		}

		if (stage == 4 && current_time - stop > 2000) next = 1;
	}
	return 0;
}