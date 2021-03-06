#include <dmzSystemFile.h>
#include <dmzFoundationReaderWriterZip.h>

#include <errno.h>
#include <string.h>
#include <time.h>
#include <unzip.h>
#include <zip.h>

/*!

\brief Test if specified file is a zip archive.
\ingroup Foundation
\param[in] FileName String containing name of file to test.
\return Returns dmz::True if the file is zip archive.

*/
dmz::Boolean
dmz::is_zip_file (const String &FileName) {

   Boolean result (False);

   FILE *fd = open_file (FileName, "rb");

   if (fd) {

      char data[4] = {0, 0, 0, 0};

      read_file (fd, 4, data);

      result = (data[0] == 0x50) && (data[1] == 0x4B) &&
         (data[2] == 0x03) && (data[3] == 0x04);

      close_file (fd); fd = 0;
   }

   return result;
}

/*!

\class dmz::ReaderZip
\ingroup Foundation
\brief Reads files from a zip archive.

*/

//! \cond
struct dmz::ReaderZip::State {

   unzFile zf;
   String zipFileName;
   String fileName;
   String error;

   State () : zf (0) {;}

   Boolean zip_error (const int Err) {

      Boolean result (Err == UNZ_OK ? True : False);

      if (!result) {

         if (Err == UNZ_ERRNO) { error.flush () << strerror (errno); }
         else if (Err == UNZ_PARAMERROR) { error.flush () << "Parameter Error."; }
         else if (Err == UNZ_BADZIPFILE) { error.flush () << "Bad Zip File."; }
         else if (Err == UNZ_INTERNALERROR) { error.flush () << "Internal Zip Error."; }
         else if (Err == UNZ_CRCERROR) { error.flush () << "CRC Zip Error."; }
         else { error.flush () << "Unknown Zip Error."; }
      }

      return result;
   }
};
//! \endcond


//! Constructor.
dmz::ReaderZip::ReaderZip () : _state (*(new State)) {;}


//! Destructor.
dmz::ReaderZip::~ReaderZip () {

   close_zip_file ();
   delete &_state;
}


// ReaderZip Interface
/*!

\brief Opens zip archive for reading.
\param[in] FileName String containing name of zip archive to open.
\param[in] Mode Parameter for future functionality.
\return Returns dmz::True if the zip archive was successfully opened.

*/
dmz::Boolean
dmz::ReaderZip::open_zip_file (const String &FileName, const UInt32 Mode) {

   Boolean result (False);

   close_zip_file ();

   _state.zf = unzOpen (FileName.get_buffer ());

   if (_state.zf) {

      _state.zipFileName = FileName;
      result = True;
   }

   return result;
}


/*!

\brief Gets the name of the currently opened zip archive.
\return Returns a String containing the name of the opened zip archive. An empty
string is returned if no zip archive is currently open.

*/
dmz::String
dmz::ReaderZip::get_zip_file_name () const { return _state.zipFileName; }


/*!

\brief Gets a list of files stored in the zip archive.
\param[out] container StringContainer used to store the list of file in the zip archive.
\return Returns dmz::True if the container was successfully populated. Returns
dmz::False if there is no open zip archive open.

*/
dmz::Boolean
dmz::ReaderZip::get_file_list (StringContainer &container) const {

   Boolean result (False);

   if (_state.zf) {

      result = True;

      int value = unzGoToFirstFile (_state.zf);

      while (result && (UNZ_OK == value)) {

         unz_file_info info;

         const int Found = unzGetCurrentFileInfo (_state.zf, &info, 0, 0, 0, 0, 0, 0);

         if (Found == UNZ_OK && (info.size_filename > 0)) {

            char *buffer = new char[info.size_filename + 1];

            if (buffer) {

               buffer[0] = '\0';

               if (UNZ_OK == unzGetCurrentFileInfo (
                     _state.zf,
                     0, // info struct
                     buffer, info.size_filename,
                     0, 0, // extra field
                     0, 0)) { // comment field

                  buffer[info.size_filename] = '\0';

                  container.add (buffer);
               }

               delete []buffer; buffer = 0;
            }
         }
         else { result = _state.zip_error (Found); }

         value = unzGoToNextFile (_state.zf);
      }
   }

   return result;
}


/*!

\brief Closes current zip archive.
\return Returns dmz::True if the zip archive was successfully closed.

*/
dmz::Boolean
dmz::ReaderZip::close_zip_file () {

   Boolean result (False);

   if (_state.zf) { 

      close_file ();

      result = _state.zip_error (unzClose (_state.zf));
   }

   _state.zf = 0;
   _state.zipFileName.flush ();

   return result;
}


// Reader Interface.

dmz::Boolean
dmz::ReaderZip::open_file (const String &FileName) {

   Boolean result (False);

   if (_state.zf) {

      close_file ();

      result = _state.zip_error (unzLocateFile (
         _state.zf,
         FileName.get_buffer (),
         1)); // Case sensitive

      if (result) {

         result = _state.zip_error (unzOpenCurrentFile (_state.zf));

         if (result) { _state.fileName = FileName; }
      }
   }
   else { _state.error.flush () << "Zip archive not open."; }

   return result;
}


dmz::String
dmz::ReaderZip::get_file_name () const { return _state.fileName; }
 

dmz::UInt64
dmz::ReaderZip::get_file_size () const {

   UInt64 result (0);
   unz_file_info info;

   if (_state.zip_error (
         unzGetCurrentFileInfo (_state.zf, &info, 0, 0, 0, 0, 0, 0))) {

      result = UInt64 (info.uncompressed_size);
   }

   return result;
}


dmz::Int32
dmz::ReaderZip::read_file (void *buffer, const Int32 Size) {

   Int32 result (0);

   if (_state.zf && buffer && (Size > 0)) {

      result = unzReadCurrentFile (_state.zf, buffer, (unsigned)Size);

      if (result < 0) { _state.zip_error (result); result = -1; }
   }

   return result;
}


dmz::Boolean
dmz::ReaderZip::close_file () {

   Boolean result (False);

   if (_state.zf) { result = _state.zip_error (unzCloseCurrentFile (_state.zf)); }

   _state.fileName.flush ();

   return result;
}


dmz::String
dmz::ReaderZip::get_error () const { return _state.error; }

/*!

\class dmz::WriterZip
\ingroup Foundation
\brief Writes files to a zip archive.

*/

//! \cond
struct dmz::WriterZip::State {

   zipFile zf;
   int level;
   String zipFileName;
   String fileName;
   String error;

   State () : zf (0), level (9) {;}

   Boolean zip_error (const int Err) {

      Boolean result (Err == ZIP_OK ? True : False);

      if (!result) {

         if (Err == ZIP_ERRNO) { error.flush () << strerror (errno); }
         else if (Err == ZIP_PARAMERROR) { error.flush () << "Parameter Error."; }
         else if (Err == ZIP_BADZIPFILE) { error.flush () << "Bad Zip File."; }
         else if (Err == ZIP_INTERNALERROR) { error.flush () << "Internal Zip Error."; }
         else { error.flush () << "Unknown Zip Error."; }
      }

      return result;
   }
};
//! \endcond

//! Constructor.
dmz::WriterZip::WriterZip () : _state (*(new State)) {;}


//! Destructor.
dmz::WriterZip::~WriterZip () {

   close_zip_file ();
   delete &_state;
}


// WriterZip Interface
/*!

\brief Creates zip archive for writing.
\details Will close any open zip archive before opening a new archive. If a zip archive
already exists with the same name, it will be overwritten.
\param[in] FileName String containing the name of the new zip archive.
\param[in] Mode Parameter for future functionality.
\return Returns dmz::True if the archive was successfully created.

*/
dmz::Boolean
dmz::WriterZip::open_zip_file (const String &FileName, const UInt32 Mode) {

   Boolean result (False);

   close_zip_file ();

   _state.zf = zipOpen (FileName.get_buffer (), APPEND_STATUS_CREATE);

   if (_state.zf) {

      _state.zipFileName = FileName;
      result = True;
   }

   return result;
}


/*!

\brief Gets the name of the current zip archive.
\return Returns a String containing the name of the open zip archive. Returns an
empty string if no archive is open.

*/
dmz::String
dmz::WriterZip::get_zip_file_name () const { return _state.zipFileName; }


/*!

\brief Closes current open zip archive.
\return Returns dmz::True if the archive is successfully closed.

*/
dmz::Boolean
dmz::WriterZip::close_zip_file () {

   Boolean result (False);

   if (_state.zf) { 

      close_file ();
      result = _state.zip_error (zipClose (_state.zf, 0));
   }

   _state.zf = 0;
   _state.zipFileName.flush ();

   return result;
}


/*!

\brief Sets the zip compression level.
\details The compress level must be between 0 and 9 with 9 being the highest compression
level while also the slowest.
\param[in] Level Specifies the compression level.
\return Returns the level being used. If the level is out of range, it is clamped to the
closest valid value.

*/
dmz::Int32
dmz::WriterZip::set_level (const Int32 Level) {

   if (Level < 0) { _state.level = 0; }
   else if (Level > 9) { _state.level = 9; }
   else { _state.level = Level; }

   return _state.level;
}


/*!

\brief Gets the current compression level.
\return Returns the current compression level. Value will be between 0 and 9.

*/
dmz::Int32
dmz::WriterZip::get_level () const { return _state.level; }


dmz::Boolean
dmz::WriterZip::open_file (const String &FileName) {

   Boolean result (False);

   if (_state.zf) {

      close_file ();

      zip_fileinfo zinfo;

      zinfo.tmz_date.tm_sec = zinfo.tmz_date.tm_min = zinfo.tmz_date.tm_hour =
      zinfo.tmz_date.tm_mday = zinfo.tmz_date.tm_min = zinfo.tmz_date.tm_year = 0;
      zinfo.dosDate = 0;
      zinfo.internal_fa = 0;
      zinfo.external_fa = 0;

      time_t tm = time (0);

      struct tm *tms = localtime (&tm);

      if (tms) {

         zinfo.tmz_date.tm_sec  = tms->tm_sec;
         zinfo.tmz_date.tm_min  = tms->tm_min;
         zinfo.tmz_date.tm_hour = tms->tm_hour;
         zinfo.tmz_date.tm_mday = tms->tm_mday;
         zinfo.tmz_date.tm_mon  = tms->tm_mon;
         zinfo.tmz_date.tm_year = tms->tm_year + 1900;
      }

      result = _state.zip_error (zipOpenNewFileInZip (
         _state.zf,
         FileName.get_buffer (),
         &zinfo,
         0, 0, 0, 0, 0, // No comments
         Z_DEFLATED,
         _state.level));

      if (result) { _state.fileName = FileName; }
   }
   else { _state.error.flush () << "Zip archive not open."; }

   return result;
}


dmz::String
dmz::WriterZip::get_file_name () const { return _state.fileName; }
 

dmz::Boolean
dmz::WriterZip::write_file (const void *Buffer, const Int32 Size) {

   Boolean result (False);

   if (_state.zf && Buffer && (Size > 0)) {

      result = _state.zip_error (zipWriteInFileInZip (
         _state.zf,
         Buffer,
         (unsigned)Size));
   }

   return result;
}


dmz::Boolean
dmz::WriterZip::close_file () {

   Boolean result (False);

   if (_state.zf) {

      if (_state.fileName) { result = _state.zip_error (zipCloseFileInZip (_state.zf)); }
      else { result = True; }
   }
   else { _state.error.flush () << "Zip archive not open."; }

   _state.fileName.flush ();

   return result;
}


dmz::String
dmz::WriterZip::get_error () const { return _state.error; }


