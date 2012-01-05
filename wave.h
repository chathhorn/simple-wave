#ifndef WAVE_H_
#define WAVE_H_

/*** Simple Wave File Parser ***/
// This file provides limited support for reading and writing MS Wave (.wav)
// files. See the "Wave" class below for details. See
// http://www.sonicspot.com/guide/wavefiles.html for more details on the MS
// Wave file format.
//
// This is free software to copy, use, modify, etc. with no conditions.
// Questions? Comments? Email me: Chris Hathhorn, chathhorn@gmail.com
//
// Example usage:
//      Wave wave;
//      wave.Load(filename);
//
//      // Set various header values.
//      wave.fmt_chunk.nchannels = 1;
//      wave.fmt_chunk.block_align = 2;
//      wave.fmt_chunk.bits_per_sample = 16;
//
//      // Resize the data chunk.
//      unsigned nsamples = 10000;
//      wave.Resize(nsamples);
//
//      // Set various sample values in the data chunk.
//      for (unsigned i = 0; i != nsamples; ++i) {
//            wave.SetSample(i, my_sample_value[i]);
//      }
//
//      wave.Save(filename);
//

//#include <stdint.h>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;

template <class T>
static void ReadLittleEndian(std::istream& stream, T& out) {
      out = 0;
      for (unsigned i = 0; i != sizeof(T); ++i) {
            char byte;
            stream.read(&byte, 1);
            out |= (unsigned char)byte << 8*i;
      }
}

template <class T>
static void WriteLittleEndian(std::ostream& stream, T const& out) {
      for (unsigned i = 0; i != sizeof(T); ++i) {
            char byte = (out >> 8*i) & 0xff;
            stream.write(&byte, 1);
      }
}

/*** Chunk ***/
// A Wave file contains, at a minimum, 3 sorts of chunks (or sections). The
// RIFF chunk, the format chunk, and the data chunk. The RIFF chunk identifies
// the file as a RIFF file and a Wave file (a sort of RIFF file). The format
// chunk includes various information about the encoding of the data (e.g., the
// number of channels, the sort of compression used [if any], the sampling
// rate), and the data chunk includes the actual data encoded according to the
// values in the format chunk. See
// http://www.sonicspot.com/guide/wavefiles.html for more details about Wave
// file layout.
class Chunk {
      public:
            /*** Public Data ***/
            uint32_t chunk_type;
            uint32_t chunk_size;

            /*** Public Methods ***/
            virtual void ReadFrom(std::istream& stream);
            virtual void WriteTo(std::ostream& stream) const;

            /*** Constants ***/
            // Actually 4 characters -- "RIFF" (but reversed).
            static uint32_t const CHUNK_TYPE_RIFF = 0x46464952;
            // "fmt "
            static uint32_t const CHUNK_TYPE_FMT = 0x20746d66;
            // "data"
            static uint32_t const CHUNK_TYPE_DATA = 0x61746164;

            static uint32_t const DEFAULT_CHUNK_SIZE_FMT = 16;
            static uint32_t const DEFAULT_CHUNK_SIZE_DATA = 0;
            
            // The default file size minus 8.
            static uint32_t const DEFAULT_CHUNK_SIZE_RIFF 
                  = 4 + 8 + DEFAULT_CHUNK_SIZE_FMT + 8 + DEFAULT_CHUNK_SIZE_DATA;

      protected:
            /*** Constructors ***/
            Chunk(uint32_t type) : chunk_type(type) { 
                  switch (chunk_type) {
                        case CHUNK_TYPE_RIFF:
                              chunk_size = DEFAULT_CHUNK_SIZE_RIFF;
                              break;
                        case CHUNK_TYPE_FMT:
                              chunk_size = DEFAULT_CHUNK_SIZE_FMT;
                              break;
                        case CHUNK_TYPE_DATA:
                              chunk_size = DEFAULT_CHUNK_SIZE_DATA;
                              break;
                        default:
                              chunk_size = 0;
                  }
            }

            Chunk(Chunk const& other) 
                  : chunk_type(other.chunk_type), chunk_size(other.chunk_size) { }
};

void Chunk::ReadFrom(std::istream& stream) {
      ReadLittleEndian(stream, chunk_size);
}

void Chunk::WriteTo(std::ostream& stream) const {
      WriteLittleEndian(stream, chunk_type);
      WriteLittleEndian(stream, chunk_size);
}

/*** RiffChunk ***/
// Identifies this file as a RIFF file and a Wave file (a sort of the more
// general RIFF type).
class RiffChunk : public Chunk {
      public:
            /*** Public Data ***/
            uint32_t riff_type;
            
            /*** Constructors ***/
            RiffChunk(void) 
                  : Chunk(CHUNK_TYPE_RIFF), riff_type(RIFF_TYPE_WAVE) { }

            /*** Public Methods ***/
            virtual void ReadFrom(std::istream& stream);
            virtual void WriteTo(std::ostream& stream) const;

            /*** Constants ***/
            // "WAVE"
            static uint32_t const RIFF_TYPE_WAVE = 0x45564157;

};

void RiffChunk::ReadFrom(std::istream& stream) {
      Chunk::ReadFrom(stream);
      ReadLittleEndian(stream, riff_type);
}

void RiffChunk::WriteTo(std::ostream& stream) const {
      Chunk::WriteTo(stream);
      WriteLittleEndian(stream, riff_type);
}

/*** FmtChunk ***/
// Contains information about how this Wave file's data is encoded.
class FmtChunk : public Chunk {
      public:
            /*** Public Data ***/
            uint16_t compression;
            uint16_t nchannels;
            uint32_t sample_rate;
            uint32_t bytes_per_sec;
            uint16_t block_align;
            uint16_t bits_per_sample;

            /*** Constructors ***/
            FmtChunk(void) 
                  : Chunk(CHUNK_TYPE_FMT),
                    compression(DEFAULT_COMPRESSION),
                    nchannels(DEFAULT_NCHANNELS),
                    sample_rate(DEFAULT_SAMPLE_RATE),
                    bytes_per_sec(DEFAULT_BYTES_PER_SEC),
                    block_align(DEFAULT_BLOCK_ALIGN),
                    bits_per_sample(DEFAULT_BITS_PER_SAMPLE)
            { }

            /*** Public Methods ***/
            virtual void ReadFrom(std::istream& stream);
            virtual void WriteTo(std::ostream& stream) const;

            /*** Constants ***/
            static uint16_t const COMPRESSION_NONE = 1;

            static uint16_t const DEFAULT_COMPRESSION = COMPRESSION_NONE;
            static uint16_t const DEFAULT_NCHANNELS = 1;
            static uint32_t const DEFAULT_SAMPLE_RATE = 22050;
            static uint32_t const DEFAULT_BITS_PER_SAMPLE = 16;
            static uint32_t const DEFAULT_BLOCK_ALIGN = (DEFAULT_BITS_PER_SAMPLE/8) * DEFAULT_NCHANNELS;
            static uint32_t const DEFAULT_BYTES_PER_SEC = DEFAULT_SAMPLE_RATE * DEFAULT_BLOCK_ALIGN;
};

void FmtChunk::ReadFrom(std::istream& stream) {
      Chunk::ReadFrom(stream);

      ReadLittleEndian(stream, compression);
      ReadLittleEndian(stream, nchannels);
      ReadLittleEndian(stream, sample_rate);
      ReadLittleEndian(stream, bytes_per_sec);
      ReadLittleEndian(stream, block_align);
      ReadLittleEndian(stream, bits_per_sample);

      if (compression != COMPRESSION_NONE) {
            std::cerr << "This WAV file appears to be compressed -- I can't deal with that." 
                      << std::endl;
      }
}

void FmtChunk::WriteTo(std::ostream& stream) const {
      Chunk::WriteTo(stream);

      WriteLittleEndian(stream, compression);
      WriteLittleEndian(stream, nchannels);
      WriteLittleEndian(stream, sample_rate);
      WriteLittleEndian(stream, bytes_per_sec);
      WriteLittleEndian(stream, block_align);
      WriteLittleEndian(stream, bits_per_sample);
}

/*** DataChunk ***/
// The actual data contained in this Wave file, i.e. the raw waveform to send
// out to the speakers. A DataChunk might also represent an unidentified chunk
// because a DataChunk basically just represents an uninterpretted swathe of
// the file of a certain length.
class DataChunk : public Chunk {
      public:
            /*** Constructors ***/
            DataChunk(void) 
                  : Chunk(CHUNK_TYPE_DATA), data_(NULL), data_length_(0) { }
            explicit DataChunk(uint32_t chunk_type) 
                  : Chunk(chunk_type), data_(NULL), data_length_(0) { }
            // Because the DataChunk actually allocates some memory (see the
            // "new" in the ReAllocData() method), we should implement the copy
            // constructor, the assignment operator, and the destructor so our
            // class plays nice, for example, with the vector<> class (which we
            // use in the Wave class below).
            DataChunk(DataChunk const& other) 
                  : Chunk(other), data_(NULL), data_length_(0) {
                  ReAllocData(other.chunk_size);
                  std::memcpy(data_, other.data_, other.data_length_);
            }

            /*** Public Methods ***/
            virtual void ReadFrom(std::istream& stream);
            virtual void WriteTo(std::ostream& stream) const;

            // Skips over the data chunk in the stream instead of reading and
            // allocating memory to store it.
            void Skip(std::istream& stream);

            void ReAllocData(unsigned length);
            char* data() const { return data_; }

            /*** Operators ***/
            DataChunk& operator=(DataChunk const& other) {
                  if (this == &other) return *this;
                  ReAllocData(other.chunk_size);
                  std::memcpy(data_, other.data_, other.data_length_);
                  return *this;
            }

            /*** Destructor ***/
            ~DataChunk(void) { 
                  if (data_) delete[] data_; 
                  data_ = NULL; 
            }

      private:
            char* data_;
            unsigned data_length_;

};

void DataChunk::ReAllocData(unsigned length) {
      if (data_) delete[] data_;
      data_length_ = chunk_size = length;

      // Chunks are word aligned, with a possible null-byte filler.
      data_length_ += chunk_size % 2;

      if (data_length_) {
            data_ = new char[data_length_];
            if (chunk_size % 2) data_[data_length_ - 1] = 0;
      }
}

void DataChunk::ReadFrom(std::istream& stream) {
      Chunk::ReadFrom(stream);
      ReAllocData(chunk_size);
      stream.read(data_, data_length_);
}

// Skips over the data chunk in the stream instead of reading and allocating
// memory to store it.
void DataChunk::Skip(std::istream& stream) {
      ReAllocData(0);
      Chunk::ReadFrom(stream);
      data_length_ = chunk_size + chunk_size%2;
      stream.ignore(data_length_);
}

void DataChunk::WriteTo(std::ostream& stream) const {
      Chunk::WriteTo(stream);
      stream.write(data_, data_length_);
}

/*** Wave ***/
// An MS Wave file parser. 
class Wave {
      public:
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
            unsigned nsamples(void) const {
                  if (!data_chunk.chunk_size || !bytes_per_sample()) {
                        return 0;
                  } else { 
                        return data_chunk.chunk_size / bytes_per_sample();
                  }
            }

            friend std::ostream& operator<<(std::ostream& stream, Wave const& wave);

      private:
            // A sample includes all channels. This would also be equal to
            // bytes_per_sample_slice() * fmt_chunk.nchannels.
            unsigned bytes_per_sample(void) const {
                  return fmt_chunk.block_align;
            }
            // A sample slice is the sample from a single channel.
            unsigned bytes_per_sample_slice(void) const {
                  return fmt_chunk.bits_per_sample / 8;
            }

            void Load(std::ifstream& file, std::string const& filename, bool load_data);
            void UpdateRiffFileSize(void);
            void UpdateFmtValues(void);

            static unsigned long long max_signed_value(unsigned sizeof_thing) {
                  return ~(1<<(sizeof_thing*8-1)) & max_thing_value(sizeof_thing);
            }

            static unsigned long long max_thing_value(unsigned sizeof_thing) {
                  unsigned long long answer = 0;
                  for (unsigned i = 0; i != sizeof_thing; ++i) {
                        answer |= (unsigned long long)0xff << 8*i;
                  }
                  return answer;
            }

            static unsigned long long GetValue(char const* things, unsigned sizeof_thing, bool thing_is_signed);
            static double TakeChannelAvg(char const* things, unsigned nthings, unsigned sizeof_thing, bool thing_is_signed);
            static void PutChannelAvg(double value, char* things, unsigned nthings, unsigned sizeof_thing, bool thing_is_signed);
};

void Wave::Resize(unsigned new_nsamples) {
      UpdateFmtValues();
      data_chunk.ReAllocData(new_nsamples * bytes_per_sample());
}

// GetSample() and SetSample() fail silently if the offset is out of bounds.
double Wave::GetSample(unsigned offset) const {
      if (offset >= nsamples()) return 0;

      char* segment = data_chunk.data() + offset * bytes_per_sample();

      if (bytes_per_sample_slice() == 1) {
            return TakeChannelAvg(segment, fmt_chunk.nchannels, bytes_per_sample_slice(), false);
      } else return TakeChannelAvg(segment, fmt_chunk.nchannels, bytes_per_sample_slice(), true);
}

void Wave::SetSample(unsigned offset, double value) {
      if (offset >= nsamples()) return;

      char* segment = data_chunk.data() + offset * bytes_per_sample();
      if (bytes_per_sample_slice() == 1) {
            PutChannelAvg(value, segment, fmt_chunk.nchannels, bytes_per_sample_slice(), false);
      } else return PutChannelAvg(value, segment, fmt_chunk.nchannels, bytes_per_sample_slice(), true);
}

// Helper function called by Load() and LoadMetadata().
void Wave::Load(std::ifstream& file, std::string const& filename, bool load_data) {

      uint32_t chunk_type;
      ReadLittleEndian(file, chunk_type);

      // Fail if the file is not a RIFF file...
      if (chunk_type != Chunk::CHUNK_TYPE_RIFF) {
            std::cerr << "Error: " << filename << " doesn't appear to be a WAV file!" 
                      << std::endl;
            return;
      }

      riff_chunk.ReadFrom(file);

      // ...or if the file is not a WAVE file.
      if (riff_chunk.riff_type != RiffChunk::RIFF_TYPE_WAVE || !file.good()) {
            std::cerr << "Error: " << filename << " doesn't appear to be a WAV file!" 
                      << std::endl;
            return;
      }

      while (file.good()) {
            ReadLittleEndian(file, chunk_type);

            // EOF only gets set when trying to read past the end of the file.
            if (!file.good()) break;

            switch (chunk_type) {
                  case Chunk::CHUNK_TYPE_FMT:
                        fmt_chunk.ReadFrom(file);
                        break;
                  case Chunk::CHUNK_TYPE_DATA:
                        if (load_data) { 
                              data_chunk.ReadFrom(file);
                        } else {
                              data_chunk.Skip(file);
                        }
                        break;
                  default:
                        DataChunk chunk(chunk_type);
                        chunk.ReadFrom(file);
                        other_chunks.push_back(chunk);
            }
      }

      file.close();
}

// Loads the contents of a .WAV file into this Wave object. Fails if the file
// doesn't exist (and prints out some messages).
void Wave::LoadMetadata(std::string const& filename) {
      std::ifstream file(filename.c_str(), std::ios_base::binary | std::ios_base::in);

      // Fail if we can't open the file...
      if (!file.good()) return; // We don't print an error here, as perverse as
                                // that may seem, because this is an expected
                                // failure in the WavSave() function if the
                                // file doesn't exist.

      Load(file, filename, false);
}


// Loads the contents of a .WAV file into this Wave object. Fails if the file
// doesn't exist (and prints out some messages).
void Wave::Load(std::string const& filename) {
      std::ifstream file(filename.c_str(), std::ios_base::binary | std::ios_base::in);

      // Fail if we can't open the file.
      if (!file.good()) {
            std::cerr << "Error: I can't open " << filename << "!" << std::endl;
            return; 
      }

      Load(file, filename, true);
}

// Writes this Wave object to a .WAV file. We create a new file if the file
// doesn't exist, otherwise we overwrite its contents.
void Wave::Save(std::string const& filename) {
      std::ofstream file(filename.c_str(), std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);

      // Fail if we can't open the file.
      if (!file.good()) {
            std::cerr << "Error: I can't open " << filename << " for writing!" 
                      << std::endl;
            return;
      }

      UpdateRiffFileSize();
      UpdateFmtValues();

      riff_chunk.WriteTo(file);
      fmt_chunk.WriteTo(file);
      
      for (std::vector<DataChunk>::iterator it = other_chunks.begin();
                  it != other_chunks.end(); ++it) {
            it->WriteTo(file);
      }

      data_chunk.WriteTo(file);

      file.close();
}

// Return the value of an arbitrary sized and signed chunk of memory of at most
// sizeof(unsigned long long) bytes.
unsigned long long Wave::GetValue(char const* things, unsigned sizeof_thing, bool thing_is_signed) {
      unsigned long long answer = 0;

      for (unsigned i = 0; i != sizeof_thing; ++i) {
            answer |= (unsigned char)things[i] << 8*i;
      }

      if (thing_is_signed) {
            if (answer > max_signed_value(sizeof_thing)) {
                  answer -= max_signed_value(sizeof_thing) + 1;
            } else {
                  answer += max_signed_value(sizeof_thing) + 1;
            }
      }

      return answer;
}

// Return the average value of all channels.
double Wave::TakeChannelAvg(char const* things, unsigned nthings, unsigned sizeof_thing, bool thing_is_signed) {
      unsigned long long total = 0;
      for (unsigned i = 0; i != nthings; ++i) {
            unsigned long long thing = GetValue(things + i * sizeof_thing, sizeof_thing, thing_is_signed);
            total += thing;
      }

      double average = total/(double)nthings;
      double munged =  (average/max_thing_value(sizeof_thing)) * 2.0 - 1.0;

      return munged;
}

// Write a value to all channels.
void Wave::PutChannelAvg(double value, char* things, unsigned nthings, unsigned sizeof_thing, bool thing_is_signed) {
      unsigned long long thing = (value+1.0)/2.0 * max_thing_value(sizeof_thing);

      if (thing_is_signed) {
            if (thing > max_signed_value(sizeof_thing)) {
                  thing -= max_signed_value(sizeof_thing) + 1;
            } else {
                  thing += max_signed_value(sizeof_thing) + 1;
            }
      }

      for (unsigned i = 0; i != nthings; ++i) {
            for (unsigned j = 0; j != sizeof_thing; ++j) {
                  things[j + i*sizeof_thing] = thing >> 8*j;
            }
      }
}

void Wave::UpdateRiffFileSize(void) {
      unsigned total = 8 * 3; // Overhead for Riff, Fmt, Data 8-byte headers;

      total += 4; // The Riff type.

      total += fmt_chunk.chunk_size;
      total += data_chunk.chunk_size;
      for (std::vector<DataChunk>::iterator it = other_chunks.begin();
                  it != other_chunks.end(); ++it) {
            total += it->chunk_size;
      }

      riff_chunk.chunk_size = total;
}

void Wave::UpdateFmtValues(void) {
      fmt_chunk.bytes_per_sec = fmt_chunk.sample_rate * fmt_chunk.nchannels * (fmt_chunk.bits_per_sample/8);
      fmt_chunk.block_align = fmt_chunk.nchannels * (fmt_chunk.bits_per_sample/8);
}

std::ostream& operator<<(std::ostream& stream, Wave const& wave) {
      return stream << "WAV file info:" << std::endl
                    << "\tFile size: " << wave.riff_chunk.chunk_size + 8 << std::endl
                    << "\tCompression: " << wave.fmt_chunk.compression << std::endl
                    << "\tChannels: " << wave.fmt_chunk.nchannels << std::endl
                    << "\tSample rate: " << wave.fmt_chunk.sample_rate << std::endl
                    << "\tBytes per second: " << wave.fmt_chunk.bytes_per_sec << std::endl
                    << "\tBlock align: " << wave.fmt_chunk.block_align << std::endl
                    << "\tBits per sample: " << wave.fmt_chunk.bits_per_sample << std::endl
                    << "\tData size: " << wave.data_chunk.chunk_size
                    << std::endl;
}

#endif
