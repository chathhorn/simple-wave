This is an MS Wave file parser intended for pedagogical purposes.

Chris Hathhorn (Fall 2011)

# `wave.h` 

This file provides an API with limited support for reading and writing MS Wave
(.wav) files. See the "Wave" class below for details. 

## The Wave class public interface

```c++
      /*** Public Data ***/
      // These objects represent the contents of this Wave file.
      RiffChunk riff_chunk;
      FmtChunk fmt_chunk;
      DataChunk data_chunk;

      // There are various other chunk types beyond those specifically
      // supported by this class. Any chunk that's not a RIFF, fmt, or
      // data chunk gets stuck here in this vector. They're represented
      // by more DataChunk objects. So a DataChunk might be the actual
      // "data" section of the file or other unrecognized sections of the
      // file. 
      std::vector<DataChunk> other_chunks;

      /*** Constructors ***/
      Wave(void) { };

      /*** Public Methods ***/
      // Load and save the Wave file. Load fails if the file doesn't
      // exist, Save will create a new file or overwrite an existing
      // file.
      void Load(std::string const& filename);
      void LoadMetadata(std::string const& filename);
      void Save(std::string const& filename);

      // Resize the data chunk to support a certain number of samples.
      // The new size of the data chunk depends on the values set in the
      // format chunk.
      void Resize(unsigned new_nsamples);

      // Get & set sample values at certain offsets, where offset <
      // nsamples(). Both fail silently if the offset is out-of-bounds.
      double GetSample(unsigned offset) const;
      void SetSample(unsigned offset, double value);

      // Gets the number of samples in the data chunk.
      unsigned nsamples(void) const;

      // Print diagnostic info about this Wave file.
      friend std::ostream& operator<<(std::ostream& stream, Wave const& wave);
```

Example usage:
```c++
     Wave wave;
     wave.Load(filename);

     // Set various header values.
     wave.fmt_chunk.nchannels = 1;
     wave.fmt_chunk.block_align = 2;
     wave.fmt_chunk.bits_per_sample = 16;

     // Resize the data chunk.
     unsigned nsamples = 10000;
     wave.Resize(nsamples);

     // Set various sample values in the data chunk.
     for (unsigned i = 0; i != nsamples; ++i) {
           wave.SetSample(i, my_sample_value[i]);
     }

     wave.Save(filename);
```

# `wave.cpp`

This file contains various example Wave-file manipulation functions using the
wave.h API with a simple, interactive user interface. 

Example functions:
  * `faster` -- Speed it up by dropping every other sample.
  * `slower` -- Slow it down by duplicating every other sample.
  * `echo` -- Create an echo effect by adding samples back in after a delay.
  * `amp_up` -- Increase the volume (amplitude) by 20%.
  * `amp_down` -- Decrease the volume (amplitude) by 20%.
  * `reverse` -- Reverse.
  * `mix` -- Mix two WAVs together. The new file should be as long as the
           longest operand file. The shorter file gets looped.
