//                  *****  FileLogger class implementation  *****

#include "file_logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <list>

FileLogger::FileLogger(const char *filename, uint32_t max_lines, uint32_t trimmed_lines)
 : Logger(), max_lines_(max_lines), trimmed_lines_(trimmed_lines), last_timestamp_(0)
{
    filename_ = new char[strlen(filename) +  1];
    strcpy(filename_, filename);
    if (max_lines_ < 500)
    {
        max_lines_ = 500;
    }
    if (trimmed_lines_ > max_lines_ - 200)
    {
        trimmed_lines_ = max_lines_ - 200;
    }
    count_lines();
}

int FileLogger::print(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    int ret = vprint(format, ap);

    va_end(ap);
    return ret;
}

int FileLogger::print_debug(int level, const char *format, ...)
{
    int ret = 0;

    if (isDebug(level))
    {
        va_list ap;
        va_start(ap, format);

        ret = vprint(format, ap);

        va_end(ap);
    }
    return ret;
}


int FileLogger::vprint(const char *format, va_list ap)
{
    time_t now;
    time(&now);
    if (last_timestamp_ != 0 && now - last_timestamp_ > 15 * 60)
    {
        printf("%s\n", timestamp(&now));
    }
    int ret = vprintf(format, ap);

    FILE *f = fopen(filename_, "a+");
    if (f)
    {
        if (last_timestamp_ != 0 && now - last_timestamp_ > 15 * 60)
        {
            fprintf(f, "%s\n", timestamp(&now));
            ++line_count_;
            last_timestamp_ = now;
        }
        ret = vfprintf(f, format, ap);
        if (ret > 0)
        {
            fseek(f, -ret, SEEK_END);
            for (int ii = 0 ;ii < ret; ii++)
            {
                if (fgetc(f) == '\n')
                {
                    ++line_count_;
                }
            }
        }
        fclose(f);
    }

    if (line_count_ > max_lines_)
    {
        trim_file();
    }

    return ret;
}

bool FileLogger::trim_file()
{
    bool ret = true;
    char linebuf[133];
    long pos = find_tail(trimmed_lines_);
    if (pos > 0)
    {
        uint32_t nl = 0;
        FILE *f = fopen(filename_, "r");
        if (f)
        {
            FILE *tmp = fopen("tmp.tmp", "w");
            if (tmp)
            {
                fseek(f, pos, SEEK_SET);
                while (fgets(linebuf, sizeof(linebuf), f))
                {
                    ret &= fprintf(tmp, "%s", linebuf) >= 0;
                    ++nl;
                }
                ret &= fclose(tmp) == 0;
            }
            fclose(f);

            if (ret)
            {
                remove(filename_);
                rename("tmp.tmp", filename_);
                printf("Reduced log file from %d to %d lines\n", line_count_, nl);
                line_count_ = nl;
            }
        }
    }
    return ret;
}

long FileLogger::find_line(int line, long from) const
{
    long ret = 0;
    uint32_t ln = 0;
    char linebuf[133];

    FILE *f = fopen(filename_, "r");
    if (f)
    {
        fseek(f, from, SEEK_SET);
        while (ln++ < line && fgets(linebuf, sizeof(linebuf), f))
        {
            ret = ftell(f);
            if (ret == -1)
            {
                ret = file_size();
                break;
            }
        }
        fclose(f);
    }
    return ret;
}

long FileLogger::find_tail(int lines) const
{
    std::list<long> lineptrs;
    int resolution = 1;
    char linebuf[133];

    FILE *f = fopen(filename_, "r");
    if (f)
    {
        lineptrs.emplace_back(ftell(f));
        while (fgets(linebuf, sizeof(linebuf), f))
        {
            if (lineptrs.size() == lines + 1)
            {
                lineptrs.pop_front();
            }
            lineptrs.emplace_back(ftell(f));
        }
        fclose(f);
    }
    return lineptrs.front();
}

uint32_t FileLogger::file_size() const
{
    struct stat sb = {0};
    stat(filename_, &sb);
    return sb.st_size;
}

void FileLogger::count_lines()
{
    line_count_ = 0;
    char linebuf[133];

    FILE *f = fopen(filename_, "r");
    if (f)
    {
        while (fgets(linebuf, sizeof(linebuf), f))
        {
            ++line_count_;
        }
        fclose(f);
    }
}

void FileLogger::initialize_timestamps()
{
    time(&last_timestamp_);
    print("Time initialized at %s\n", timestamp(&last_timestamp_));
}

const char *FileLogger::timestamp(const time_t *ts) const
{
    static char timbuf[32];
    strftime(timbuf, sizeof(timbuf), "%c %Z", localtime(ts));
    return timbuf;
}
