// This file contains various example Wave-file manipulation functions using
// the wave.h API with a simple, interactive user interface.
//
// This is free software to copy, use, modify, etc. with no conditions.
// Questions? Comments? Email me: Chris Hathhorn, chathhorn@gmail.com
//

#include "wave.h"
#include <string>
#include <iostream>

using namespace std;

// Functions for manipulating .WAV files. See below for more details.
double* WavLoad(string const& filename);
int WavLength(string const& filename);
void WavSave(string const& filename, double const* samples, int nsamples);

// Speed it up by dropping every other sample.
void faster(string const& filename, string const& result) {
      int old_samples_length = WavLength(filename);
      double* old_samples = WavLoad(filename);

      int new_samples_length = old_samples_length / 2;
      double* new_samples = new double[new_samples_length];

      for (int i = 0; i != new_samples_length; ++i) {
            new_samples[i] = old_samples[i*2];
      }

      WavSave(result, new_samples, new_samples_length);

      delete[] new_samples;
      delete[] old_samples;
}

// Slow it down by duplicating every other sample.
void slower(string const& filename, string const& result) {
      int old_samples_length = WavLength(filename);
      double* old_samples = WavLoad(filename);

      int new_samples_length = old_samples_length * 2;
      double* new_samples = new double[new_samples_length];

      for (int i = 0; i != new_samples_length; ++i) {
            new_samples[i] = old_samples[i/2];
      }

      WavSave(result, new_samples, new_samples_length);

      delete[] new_samples;
      delete[] old_samples;
}

// Create an echo effect by adding samples back in after a delay.
void echo(string const& filename, string const& result) {
      int old_samples_length = WavLength(filename);
      double* old_samples = WavLoad(filename);

      // Number of samples before the echo.
      int echo_delay = 10000;

      // Echo intensity.
      double echo_intensity = 0.8;

      int new_samples_length = old_samples_length + echo_delay;
      double* new_samples = new double[new_samples_length];

      for (int i = 0; i != new_samples_length; ++i) {
            // We must cover 3 cases here:
            //       1. We're at least echo_delay samples into the file, so we
            //       can be sure that i-echo_delay will still land us inside
            //       the file and i is still within the bounds of the
            //       old_samples array. In this case, our new sample is the
            //       average of the old sample and the sample from echo_delay
            //       samples ago.
            if (i >= echo_delay && i < old_samples_length) {
                  new_samples[i] = (old_samples[i] 
                        + echo_intensity * old_samples[i-echo_delay])/2;
            //       2. We're at least echo_delay samples into the file and i
            //       has outrun the length of the old_samples array. In this
            //       case, the new sample will just be echo, i.e. the sample
            //       from echo_delay samples ago.
            } else if (i >= echo_delay && i >= old_samples_length) {
                  new_samples[i] = echo_intensity * old_samples[i-echo_delay];
            //       3. Else we're not echo_delay samples into the file yet.
            //       Therefore, the new sample will just be the old sample
            //       because we've got nothing to echo yet.
            } else {
                  new_samples[i] = old_samples[i];
            }
      }

      WavSave(result, new_samples, new_samples_length);

      delete[] new_samples;
      delete[] old_samples;
}

// Increase the volume (amplitude) by 20%.
void amp_up(string const& filename, string const& result) {
      int old_samples_length = WavLength(filename);
      double* old_samples = WavLoad(filename);

      double factor = 1.2;

      for (int i = 0; i != old_samples_length; ++i) {
            old_samples[i] = factor * old_samples[i];
      }

      WavSave(result, old_samples, old_samples_length);

      delete[] old_samples;
}

// Decrease the volume (amplitude) by 20%.
void amp_down(string const& filename, string const& result) {
      int old_samples_length = WavLength(filename);
      double* old_samples = WavLoad(filename);

      double factor = 0.8;

      for (int i = 0; i != old_samples_length; ++i) {
            old_samples[i] = factor * old_samples[i];
      }

      WavSave(result, old_samples, old_samples_length);

      delete[] old_samples;
}

// Reverse.
void reverse(string const& filename, string const& result) {
      int old_samples_length = WavLength(filename);
      double* old_samples = WavLoad(filename);

      int new_samples_length = old_samples_length;
      double* new_samples = new double[new_samples_length];

      for (int i = 0; i != new_samples_length; ++i) {
            new_samples[i] = old_samples[old_samples_length - 1 - i];
      }

      WavSave(result, new_samples, new_samples_length);

      delete[] new_samples;
      delete[] old_samples;
}

// Mix two WAVs together. The new file should be as long as the longest operand
// file. The shorter file gets looped.
void mix(string const& file1, string const& file2, string const& result) {
      int file1_length = WavLength(file1);
      int file2_length = WavLength(file2);

      double* file1_samples = WavLoad(file1);
      double* file2_samples = WavLoad(file2);

      int longest_length;
      int shortest_length;
      double* longest_samples;
      double* shortest_samples;

      // Figure out which file is longest.
      if (file1_length > file2_length) {
            longest_length = file1_length;
            longest_samples = file1_samples;

            shortest_length = file2_length;
            shortest_samples = file2_samples;
      } else {
            longest_length = file2_length;
            longest_samples = file2_samples;

            shortest_length = file1_length;
            shortest_samples = file1_samples;
      }

      for (int i = 0; i != longest_length; ++i) {
            // The new sample will be the average of the sample from both
            // files. The expression "i % shortest_length" returns a value
            // between 0 and shortest_length-1, ensuring that we never read
            // beyond the bounds of shortest_samples.
            longest_samples[i] = (longest_samples[i] + shortest_samples[i%shortest_length]) / 2;
      }

      WavSave(result, longest_samples, longest_length);

      delete[] file1_samples;
      delete[] file2_samples;
}

int main(int argc, char** argv) {

      cout << "This here is an interactive program for manipulating MS Wave files." << endl;

      cout << "Usage: <mode> <input WAV(s)> <output WAV>" << endl
           << "Where mode can be one of the following:" << endl
           << "\t f : Faster." << endl
           << "\t s : Slower." << endl
           << "\t e : Echo." << endl
           << "\t r : Reverse." << endl
           << "\t + : Plus volume." << endl
           << "\t - : Minus volume." << endl
           << "\t m : Mix two .WAV files together. Takes an extra filename argument." << endl
           << "\t q : Quit." << endl
           << endl;

      // An infinite loop. Exits when the user enters 'q'.
      for (;;) {
            cout << "> ";

            char mode;
            cin >> mode;

            string input_file, input_file2, output_file;
            switch (mode) {
                  case 'f':
                        cin >> input_file >> output_file;
                        cout << "Faster!" << endl;
                        faster(input_file, output_file);
                        break;
                  case 's':
                        cin >> input_file >> output_file;
                        cout << "Slower!" << endl;
                        slower(input_file, output_file);
                        break;
                  case 'e':
                        cin >> input_file >> output_file;
                        cout << "Echo!" << endl;
                        echo(input_file, output_file);
                        break;
                  case 'r':
                        cin >> input_file >> output_file;
                        cout << "Reverse!" << endl;
                        reverse(input_file, output_file);
                        break;
                  case '+':
                        cin >> input_file >> output_file;
                        cout << "Increase volume!" << endl;
                        amp_up(input_file, output_file);
                        break;
                  case '-':
                        cin >> input_file >> output_file;
                        cout << "Decrease volume!" << endl;
                        amp_down(input_file, output_file);
                        break;
                  case 'm':
                        cin >> input_file >> input_file2 >> output_file;
                        cout << "Mix!" << endl;
                        mix(input_file, input_file2, output_file);
                        break;
                  case 'q':
                        cout << "Exiting." << endl;
                        return 0;
                  default:
                        cout << "Unknown mode: " << mode << endl
                             << "Use 'q' to quit." << endl;
                        cin.clear();
            }

            cout << endl;
      }

      return 0;
}

// Reads and returns the sample values from a .WAV file. We return the samples
// as a series of values between +1.0 and -1.0. Use WavLength() to get the
// number of values returned. This functions allocates memory to store its
// return value, so use delete[] to free this memory.
double* WavLoad(string const& filename) {
      Wave wave;
      wave.Load(filename);

      int nsamples = wave.nsamples();
      double* samples = new double[nsamples];

      for (int i = 0; i != nsamples; ++i) {
            samples[i] = wave.GetSample(i);
      }

      return samples;
}

// Returns the length of the "array" of data values returned by WavLoad().
int WavLength(string const& filename) {
      Wave wave;
      wave.LoadMetadata(filename);
      return wave.nsamples();
}

// Writes or overwrites a .WAV file with the parameter sample values. We expect
// the samples to be in the range -1.0 to +1.0, like those returned by
// WavLoad().
void WavSave(string const& filename, double const* samples, int nsamples) {
      Wave wave;

      // If the Wave file exists, then let's save as much metadata as possible
      // from it.
      wave.LoadMetadata(filename);

      // If we have more than one channel, the Wave class will just mirror the
      // same sample data across all channels, doubling (at least) the size of
      // the file for no reason.
      wave.fmt_chunk.nchannels = 1;

      wave.Resize(nsamples);

      for (int i = 0; i != nsamples; ++i) {
            wave.SetSample(i, samples[i]);
      }

      wave.Save(filename);
}
