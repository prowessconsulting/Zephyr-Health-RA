/* heartrate.c - source file for measuring heartrate */

/*
 * This implementation includes the source code from PulseSensor.com,
 * which is licensed under MIT license.
 * https://github.com/WorldFamousElectronics/PulseSensor_Amped_Arduino
 */

#include "heartrate.h"

uint32_t IBI = 600;			// value holds the time interval between beats! Must be seeded!
bool Pulse = false;     		// "True" when User's live heartbeat is detected. "False" when not a "live beat".
uint32_t rate[10];			// array to hold last ten IBI values
uint32_t lastBeatTime = 0;		// used to find IBI
uint32_t P = 2048;			// used to find peak in pulse wave, seeded
uint32_t T = 2048;			// used to find trough in pulse wave, seeded
uint32_t thresh = 2100;			// used to find instant moment of heart beat, seeded
uint32_t amp = 400;			// used to hold amplitude of pulse waveform, seeded
bool firstBeat = true;			// used to seed rate array so we startup with reasonable BPM
bool secondBeat = false;		// used to seed rate array so we startup with reasonable BPM

int measure_heartrate(uint32_t Signal)
{
	int HR = 0;
	int N = k_uptime_get_32() - lastBeatTime;	// monitor the time since the last beat to avoid noise

	// find the peak and trough of the pulse wave
	if(Signal < thresh && N > (IBI/5)*3) {	// avoid dichrotic noise by waiting 3/5 of last IBI
		if (Signal < T) {		// T is the trough
			T = Signal;		// keep track of lowest point in pulse wave
		}
	}

	if(Signal > thresh && Signal > P){	// thresh condition helps avoid noise
		P = Signal;			// P is the peak
	}					// keep track of highest point in pulse wave

	// NOW IT'S TIME TO LOOK FOR THE HEART BEAT
	// signal surges up in value every time there is a pulse
	if (N > 250 && N < 2000) {				// avoid noise. 30 bpm < possible human heart beat < 240
		if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) ){
			Pulse = true;				// set the Pulse flag when we think there is a pulse
			IBI = k_uptime_get_32() - lastBeatTime;	// measure time between beats in mS
			lastBeatTime = k_uptime_get_32();		// keep track of time for next pulse

			if(secondBeat) {			// if this is the second beat, if secondBeat == TRUE
				secondBeat = false;		// clear secondBeat flag
				for(int i=0; i<=9; i++) {	// seed the running total to get a realisitic BPM at startup
					rate[i] = IBI;
				}
			}

			if(firstBeat) {				// if it's the first time we found a beat, if firstBeat == TRUE
				firstBeat = false;		// clear firstBeat flag
				secondBeat = true;		// set the second beat flag
				return 0;			// IBI value is unreliable so discard it
			}


			// keep a running total of the last 10 IBI values
			int runningTotal = 0;			// clear the runningTotal variable

			for(int i=0; i<=8; i++) {		// shift data in the rate array
				rate[i] = rate[i+1];		// and drop the oldest IBI value
				runningTotal += rate[i];	// add up the 9 oldest IBI values
			}

			rate[9] = IBI;				// add the latest IBI to the rate array
			runningTotal += rate[9];		// add the latest IBI to runningTotal
			runningTotal /= 10;			// average the last 10 IBI values
			HR = 30000/runningTotal;		// how many beats can fit into a minute? that's BPM!
		}
	}

	if (Signal < thresh && Pulse == true) {	// when the values are going down, the beat is over
		Pulse = false;			// reset the Pulse flag so we can do it again
		amp = P - T;			// get amplitude of the pulse wave
		thresh = (amp * 2 / 3) + T;	// set thresh at 67% of the amplitude
		P = thresh;			// reset these for next time
		T = thresh;
	}

	if (N > 2500) {				// if 2.5 seconds go by without a beat
		thresh = 2048;			// set thresh default
		P = 2048;			// set P default
		T = 2048;			// set T default
		lastBeatTime = k_uptime_get_32();	// bring the lastBeatTime up to date
		firstBeat = true;		// set these to avoid noise
		secondBeat = false;		// when we get the heartbeat back
	}
	return HR;
}
