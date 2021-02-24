/*
 * user_io.c
 *
 *  Created on: Feb 14, 2021
 *      Author: cooper
 *  Description: This file aims to have all the necessary code for
 *  the User IO subsystem.
 */

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#define WAVEFORM_SIZE 1746

// Private vars
uint16_t waveform[WAVEFORM_SIZE] = {0};
uint16_t input_test[800] = {0};
int sum = 0;

// private declarations
void reset_counters() {
}

int find_location(int x) {
	int tmp = 2 * (x+1);
	return tmp + tmp/10 - tmp/100 -1;
}

void normalize_waveform() {
	// Now we start removing DC offset as best we can.
	// also I know this has potential for weird maths bc of the unsigned/signed discrepancy.
	printf("sum: %d\n", sum);
	int16_t average_whole = sum/1746 - (2^15);
	uint16_t average_remainder;
	// avoid divide by zero
	if((sum&0x8000) == 0)
		average_remainder = 0;
	else
		average_remainder = 1746 / (sum & 0x8000);
	uint8_t multiplier = 1;
	// Remove the whole number offset from the waveform.
	for(uint16_t i = 0; i < WAVEFORM_SIZE; i++) {
		// We don't want to underflow.
		if(waveform[i] > average_whole * multiplier) {
			waveform[i] = waveform[i] - average_whole * multiplier;
			multiplier = 1;
		} else {
			multiplier++;
		}
	}
	// just return early if there is no remainder (unlikely).
	if(average_remainder == 0) return;
	// Do our best to remove fractional part
	// I may have to get more precise by finding the remainder of the remainder division even.
	multiplier = 1;
	for(uint16_t i = 0; i < WAVEFORM_SIZE; i++) {
		if(i % average_remainder == 0) {
			if(waveform[i] >= 1) {
				waveform[i] = waveform[i] - multiplier;
				multiplier = 1;
			}
			else {
				multiplier++;
			}
		}
	}
}

void stream_touch_sample(uint16_t x, uint16_t y) {
	// This is the math necessary to expand 800 samples into 1746.
	// Need to manually add 2 points at the end to make it match the beginning.
	int location = find_location(x);

	// The y math scales the 480 pixels to be [0, 2^16-1] as per the specified interface.
	waveform[location] = 136 * y + 17 * y/32;
}

void interpolate_waveform() {
	reset_counters();
	// So we double every sample, by adding 1 sample before it
	// Next, every 10 samples we add a new one, skipping every 100 samples
	uint16_t value_before = waveform[WAVEFORM_SIZE-1];
	uint16_t value_after;
	uint8_t inc = 1;
	sum = 0;

	// It is easier to loop over every point given and interpolate between
	int alocation = find_location(0);
	int blocation = find_location(799);
	int num = 2;
	value_before = waveform[blocation];
	value_after = waveform[alocation];
	int index = 0;
	for(int i = 0; i < 800; i++) {
		for(int j = 1; j < num; j++) {
			waveform[index] = (j * (value_after - value_before))/num + value_before;
			sum += waveform[index];
			index++;
		}
		index++;
		blocation = alocation;
		alocation = find_location((i+1)%800);
		num = alocation - blocation;
		value_before = waveform[blocation];
		value_after = waveform[alocation];
	}
	value_after = waveform[find_location(0)];
	value_before = waveform[find_location(799)];
	// add final two points to smooth towards the beginning.
	waveform[WAVEFORM_SIZE-2] = (value_after - value_before) / 3 + value_before;
	waveform[WAVEFORM_SIZE-1] = (2*(value_after - value_before)) / 3 + value_before;
	waveform[0] = waveform[WAVEFORM_SIZE-1];
}

void print_waveform(FILE *fp, uint16_t *x, int size) {
	for(int i = 0; i < size-1; i++) {
		fprintf(fp, "%u,", x[i]);
	}
	fprintf(fp, "%u\n", x[size-1]);
}

void run_test(FILE * fp) {
	for(int i = 0; i < 800; i++) {
		stream_touch_sample(i, input_test[i]);
	}

	interpolate_waveform();

	print_waveform(fp, waveform, WAVEFORM_SIZE);
}

void main() {
	FILE *fp;

	fp = fopen("tests.txt", "w");
	for(int i = 0; i < WAVEFORM_SIZE-1; i++)
		fprintf(fp, "%d,", i);
	fprintf(fp, "%d\n", WAVEFORM_SIZE-1);

	// Test 1
	for(int i = 0; i < 800; i++) { 
		input_test[i] = i*480/800;
	}

	print_waveform(fp, input_test, 800);

	run_test(fp);

	normalize_waveform();
	print_waveform(fp, waveform, WAVEFORM_SIZE);

	// 2
	for(int i = 0; i < 800; i++) {
		input_test[i] = 240 + 239 * sin(2*M_PI * i / 800);
	}

	print_waveform(fp, input_test, 800);

	run_test(fp);

	normalize_waveform();
	print_waveform(fp, waveform, WAVEFORM_SIZE);

	// 3
	for(int i = 0; i < 800; i++) {
		input_test[i] = 120 + 120 * sin(2*M_PI * i / 800);
	}

	print_waveform(fp, input_test, 800);

	run_test(fp);

	normalize_waveform();
	print_waveform(fp, waveform, WAVEFORM_SIZE);
	fclose(fp);
}

