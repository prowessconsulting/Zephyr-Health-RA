#include <max30100_beat_detector.h>

BeatDetectorState max30100_beat_detector_state = BEATDETECTOR_STATE_INIT;
float threshold = BEATDETECTOR_MIN_THRESHOLD;
float beatPeriod = 0;
float lastMaxValue = 0;
uint64_t tsLastBeat = 0;

bool max30100_beat_detector_sample(float ir_sample_f32)
{
    bool beatDetected = false;
    switch (max30100_beat_detector_state)
    {
    case BEATDETECTOR_STATE_INIT:
        if (time > BEATDETECTOR_INIT_HOLDOFF)
        {
            max30100_beat_detector_state = BEATDETECTOR_STATE_WAITING;
        }
        break;

    case BEATDETECTOR_STATE_WAITING:
        if (ir_sample_f32 > threshold)
        {
            threshold = min(ir_sample_f32, BEATDETECTOR_MAX_THRESHOLD);
            max30100_beat_detector_state = BEATDETECTOR_STATE_FOLLOWING_SLOPE;
        }

        // Tracking lost, resetting
        if ((time - tsLastBeat) > BEATDETECTOR_INVALID_READOUT_DELAY)
        {
            beatPeriod = 0;
            lastMaxValue = 0;
        }

        max30100_beat_detector_decrease_threshold();
        break;

    case BEATDETECTOR_STATE_FOLLOWING_SLOPE:
        if (ir_sample_f32 < threshold)
        {
            max30100_beat_detector_state = BEATDETECTOR_STATE_MAYBE_DETECTED;
        }
        else
        {
            threshold = min(ir_sample_f32, BEATDETECTOR_MAX_THRESHOLD);
        }
        break;

    case BEATDETECTOR_STATE_MAYBE_DETECTED:
        if ((ir_sample_f32 + BEATDETECTOR_STEP_RESILIENCY) < threshold)
        {
            // Found a beat
            beatDetected = true;
            lastMaxValue = ir_sample_f32;
            max30100_beat_detector_state = BEATDETECTOR_STATE_MASKING;
            float delta = time - tsLastBeat;
            if (delta)
            {
                beatPeriod = BEATDETECTOR_BPFILTER_ALPHA * delta +
                             (1 - BEATDETECTOR_BPFILTER_ALPHA) * beatPeriod;
            }

            tsLastBeat = time;
        }
        else
        {
            max30100_beat_detector_state = BEATDETECTOR_STATE_FOLLOWING_SLOPE;
        }
        break;

    case BEATDETECTOR_STATE_MASKING:
        if ((time - tsLastBeat) > BEATDETECTOR_MASKING_HOLDOFF)
        {
            max30100_beat_detector_state = BEATDETECTOR_STATE_WAITING;
        }
        max30100_beat_detector_decrease_threshold();
        break;
    }

    return beatDetected;
}

float max30100_beat_detector_get_rate()
{
    if (beatPeriod != 0)
    {
        return 1.0f / beatPeriod * 1000 * 60;
    }
    else
    {
        return 0;
    }
}

void max30100_beat_detector_decrease_threshold()
{
    // When a valid beat rate readout is present, target the
    if (lastMaxValue > 0 && beatPeriod > 0)
    {
        threshold -= lastMaxValue * (1 - BEATDETECTOR_THRESHOLD_FALLOFF_TARGET) /
                     (beatPeriod / BEATDETECTOR_SAMPLES_PERIOD);
    }
    else
    {
        // Asymptotic decay
        threshold *= BEATDETECTOR_THRESHOLD_DECAY_FACTOR;
    }

    if (threshold < BEATDETECTOR_MIN_THRESHOLD)
    {
        threshold = BEATDETECTOR_MIN_THRESHOLD;
    }
}

void max30100_beat_detector_reset()
{
    beatPeriod = 0;
    lastMaxValue = 0;
    max30100_beat_detector_state = BEATDETECTOR_STATE_INIT;
}