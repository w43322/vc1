#include "kd/stream/file_reader.h"
#include "kdcore/kd_memory.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// Default token delimiters; only the 9 characters are copied, the array tail
// stays zero.
// GLOBAL: VC1 0x004d7b94
extern const char g_streamDelimiters[] = " \t\n\r;:,.?";
// Inferred name. KdFileReader::Fill reads in multiples of this. Never written,
// but the original loads it instead of folding the 4 (volatile keeps that).
// GLOBAL: VC1 0x004e99b4
volatile unsigned g_streamBlockSize = 4;

// RET 4. Base Fill: no source, so only what is left in the range.
// FUNCTION: VC1 0x0040e3c0
unsigned KdBufferStream::Fill(unsigned want)
{
    (void)want;
    return size - pos;
}

// EDI=this. Called by the deleting destructor (0x0042e3a0) and inline in
// KdFont::Load: source (Close, then its name), then buffer.
// FUNCTION: VC1 0x00419b30
__declspec(noinline) KdFileReader::~KdFileReader() throw()
{
}

// ECX=this. Slot 1 of KdFileWriter's vftable.
// FUNCTION: VC1 0x004b8780
void KdBufferOutStream::Clear()
{
    begin = 0;
    size = 0;
    pos = 0;
}

// Slot 1 of both stream vftables.
// FUNCTION: VC1 0x004b8790
void KdBufferStream::Clear()
{
    begin = 0;
    size = 0;
    pos = 0;
}

// ESI=this. Skips bytes found in the first 15 delimiters, refilling one byte
// at a time; at the end of input sets kStreamEof | kStreamFail and returns 0.
// Otherwise the first non-delimiter (already consumed).
// FUNCTION: VC1 0x004ba630
char KdBufferStream::SkipBlanks()
{
    char c;
    unsigned left;

    c = delimiters[0];
    while (memchr(delimiters, c, 15)) {
        if (size - pos == 0 && Fill(1) == 0) {
            left = size - pos;
            if (left < 1) {
                state |= (left == 0 ? kStreamEof : 0) | kStreamFail;
            }
            return 0;
        }
        c = reinterpret_cast<char*>(begin)[pos];
        pos += 1;
    }
    return c;
}

// ECX=this, EAX=out. One token: up to a delimiter, or between matching '"' /
// '\'' quotes (delimiters kept). A backslash escape rewrites the stored
// backslash (t n r b E " ' \\, anything else as is).
// FUNCTION: VC1 0x004ba6b0
void KdBufferStream::ReadToken(KdString& out)
{
    char quote;
    char c;
    char prev;

    out.Resize(0);
    quote = 0;
    c = SkipBlanks();
    if (c == 0) {
        return;
    }
    if (c == '"' || c == '\'') {
        quote = c;
    }
    else {
        out.Append(c);
    }
    for (;;) {
        if (size - pos == 0 && Fill(1) == 0) {
            return;
        }
        prev = c;
        c = reinterpret_cast<char*>(begin)[pos];
        pos += 1;
        if (quote == 0 && memchr(delimiters, c, 15)) {
            return;
        }
        if (prev == '\\') {
            switch (c) {
            case 't':
                c = '\t';
                break;
            case 'n':
                c = '\n';
                break;
            case 'r':
                c = '\r';
                break;
            case 'b':
                c = '\b';
                break;
            case 'E':
                c = 0x1b;
                break;
            case '"':
                c = '"';
                break;
            case '\'':
                c = '\'';
                break;
            case '\\':
                c = '\\';
                break;
            }
            out.data()[out.size - 1] = c;
        }
        else {
            if (c == quote) {
                return;
            }
            out.Append(c);
        }
    }
}

// EBX=this, RET 4. ReadToken with '\n' as the only delimiter; the delimiter
// set is restored afterwards.
// FUNCTION: VC1 0x004ba8c0
__declspec(noinline) KdBufferStream& KdBufferStream::ReadLine(KdString& out)
{
    // The set is saved as a range copy and cleared as a range fill (each
    // behind a first != last check), then restored by one struct assignment.
    struct DelimiterSet { char c[0x10]; } saved;
    char* const first = delimiters;
    char* const last = delimiters + sizeof(delimiters);

    if (first != last) {
        KdMemcpy(saved.c + (first - delimiters), first, static_cast<unsigned>(last - first));
    }
    if (first != last) {
        memset(first, 0, last - first);
    }
    delimiters[0] = '\n';
    ReadToken(out);
    *reinterpret_cast<DelimiterSet*>(delimiters) = saved;
    return *this;
}

// this on the stack, EBX=data, EDI=length, RET 4. driver->write(driver, rest,
// left, 1, native) until everything is written. Returns the bytes written;
// a short write sets kStreamFail, zero bytes kStreamEof as well; no file sets
// errno 0x11 too.
// FUNCTION: VC1 0x004bc2d0
unsigned KdFileSource::Write(const void* data, unsigned length)
{
    unsigned left = length;
    int count;

    while (left) {
        if (!file) {
            errno = 0x11;
            state |= kStreamEof | kStreamFail;
            return length - left;
        }
        count = file->driver->write(file->driver,
            static_cast<const char*>(data) + (length - left), left, 1, file->native);
        if (count < 1) {
            if (count == 0) {
                state |= kStreamEof | kStreamFail;
                return length - left;
            }
            state |= kStreamFail;
            break;
        }
        left -= count;
    }
    return length - left;
}

// EBX=this, EDI=buffer, ESI=length. driver->read(driver, buffer, 1, length,
// native), retried while negative. Zero bytes sets kStreamEof | kStreamFail;
// no file sets errno 0x11 as well.
// FUNCTION: VC1 0x004bc350
__declspec(noinline) unsigned KdFileSource::Read(void* buffer, unsigned length)
{
    int count;

    do {
        if (!file) {
            errno = 0x11;
            state |= kStreamEof | kStreamFail;
            return 0;
        }
        count = file->driver->read(file->driver, buffer, 1, length, file->native);
    } while (count < 0);
    if (count > 0) {
        return static_cast<unsigned>(count);
    }
    if (count == 0) {
        state |= kStreamEof | kStreamFail;
    }
    return 0;
}

// EAX=this. A failed driver close sets kStreamError | kStreamFail; the name is
// emptied in place.
// FUNCTION: VC1 0x004bc3e0
void KdFileSource::Close()
{
    KdFileHandle* handle;
    int result;

    handle = file;
    if (handle) {
        result = handle->driver->close(handle->driver, handle->native);
        free(handle);
        if (result) {
            state |= kStreamError | kStreamFail;
        }
    }
    file = 0;
    name.Resize(0);
}

// ESI=this, EAX=path, RET 4. Stores the path, flags a missing handle as an
// error, closes the previous file (which also empties the name) and takes
// the new handle.
// FUNCTION: VC1 0x004bc460
__declspec(noinline) void KdFileSource::Attach(const char* path, KdFileHandle* handle)
{
    if (!path) {
        path = reinterpret_cast<const char*>(&g_kdLogTag);
    }
    AssignCountedString(&name, path, static_cast<unsigned>(strlen(path)));
    state = 0;
    if (!handle) {
        state = kStreamError;
    }
    Close();
    file = handle;
}

// ESI=this, returns this. Writes out the buffer while the stream is clean,
// then flushes the file; the source state becomes the stream state.
// FUNCTION: VC1 0x004bc8f0
KdFileWriter& KdFileWriter::Flush()
{
    state = 0;
    source.state = 0;
    while (state == 0 && pos != 0 && Reserve(size - pos) != 0) {
    }
    if (!source.file) {
        errno = 0x11;
        source.state |= kStreamFail;
    }
    else if (source.file->driver->flush(source.file->driver, source.file->native)) {
        source.state |= kStreamFail;
    }
    state = source.state;
    source.state = state;
    return *this;
}

// RET 4. Unless at end of file: drops consumed bytes in block-size steps,
// grows the buffer when fewer than want bytes fit, then reads until want more
// bytes are buffered or the source stops. Returns the bytes now available.
// FUNCTION: VC1 0x004bc980
unsigned KdFileReader::Fill(unsigned want)
{
    unsigned chunk;
    int needed;
    unsigned discard;
    unsigned remainder;
    unsigned kept;
    unsigned end;
    unsigned target;
    unsigned count;
    unsigned block;

    if (state & kStreamEof) {
        return size - pos;
    }
    chunk = want;
    if (buffer.size >> 1 >= want) {
        chunk = buffer.size >> 1;
    }
    needed = static_cast<int>(chunk - (buffer.size - pos));
    discard = needed < 0 ? 0 : static_cast<unsigned>(needed);
    block = g_streamBlockSize;
    remainder = discard % block;
    if (remainder) {
        discard = discard - remainder + block;
    }
    if (pos < discard) {
        discard = pos;
    }
    EraseCountedBytes(buffer.begin, reinterpret_cast<char*>(buffer.begin) + discard,
        reinterpret_cast<char*>(buffer.begin) + buffer.size);
    kept = pos - discard;
    end = kept;
    if (buffer.size - kept < want) {
        buffer.Resize(kept + chunk);
        Clear();
        begin = buffer.begin;
        size = 0;
    }
    target = kept + want;
    count = 1;
    while (end < target && count != 0 && source.state == 0) {
        count = source.Read(reinterpret_cast<char*>(buffer.begin) + end, buffer.size - end);
        end += count;
    }
    state = source.state;
    source.state = state;
    reinterpret_cast<char*>(buffer.begin)[end] = 0;
    Clear();
    begin = buffer.begin;
    size = end;
    pos = kept;
    return end - kept;
}

// RET 4. Room for want more bytes: unless at end of file, when they would
// not fit the window or the buffer could not hold them anyway, writes the
// pending bytes to the file first and drops what was written; then grows
// the buffer if still short. Returns the room left.
// FUNCTION: VC1 0x004bcac0
unsigned KdFileWriter::Reserve(unsigned want)
{
    if (!(state & kStreamEof) && (want <= size - pos ||
            want >= buffer.capacity - (buffer.capacity != 0) - pos)) {
        const unsigned written = source.Write(begin, pos);
        pos -= written;
        state = source.state;
        source.state = state;
        EraseCountedBytes(begin, reinterpret_cast<char*>(begin) + written,
            reinterpret_cast<char*>(begin) + size);
        if (size - pos < want) {
            KdBufferOutStream::Reserve(want);
        }
        return size - pos;
    }
    return KdBufferOutStream::Reserve(want);
}

// ECX=this. Flushes, then the members: source (Close, then its name) and
// buffer.
// FUNCTION: VC1 0x004bcb50
KdFileWriter::~KdFileWriter() throw()
{
    Flush();
}

// RET 8 (this and path on the stack). Opens path "rt" through OpenKdFile into
// a zero-filled 0xff-byte buffer.
// FUNCTION: VC1 0x004bcbe0
KdFileReader::KdFileReader(const char* path) throw()
    : buffer(0xff, 0)
{
    // STRING: VC1 0x004d5bc8
    source.Attach(path, static_cast<KdFileHandle*>(OpenKdFile(path, "rt")));
    state = source.state;
    source.state = state;
    Clear();
    begin = buffer.begin;
    size = 0;
}

// ECX=this, RET 4. Grows the buffer to pos + want bytes and makes all of it
// the window, keeping pos; still short sets kStreamFail (and kStreamEof when
// nothing fits). Returns the room left.
// FUNCTION: VC1 0x004bdcc0
unsigned KdBufferOutStream::Reserve(unsigned want)
{
    const unsigned written = pos;

    if (size - written < want) {
        const unsigned target = want + written;
        GrowCountedBuffer(&buffer, static_cast<int>(target), 0);
        buffer.Resize(target);
        Clear();
        begin = buffer.begin;
        size = buffer.size;
        pos = written;
    }
    if (size - pos < want) {
        state |= (size - pos == 0 ? kStreamEof : 0) | kStreamFail;
    }
    return size - pos;
}

}
