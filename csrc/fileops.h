#ifndef __fileops_h__
#define __fileops_h__

typedef text TypedFile;

        string ErrorString(integer e);
        void OpenForRead(untyped_file& f, longint l);
        void OpenForRead(TypedFile& f);
        void OpenForRead(text& f);
        void OpenForWrite(untyped_file& f, longint l);
        void OpenForWrite(TypedFile& f);
        void OpenForWrite(text& f);

const integer FILE_READ_ONLY = 0;
const integer FILE_WRITE_ONLY = 1;
const integer FILE_READ_WRITE = 2;

#endif
