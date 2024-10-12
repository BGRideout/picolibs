//                  *****  FileLogger class  *****

#ifndef FILE_LOGGER_H
#define FILE_LOGGER_H

#include "logger.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>

class FileLogger : public Logger
{
private:
    char        *filename_;             // Log file name
    uint32_t    max_lines_;             // Maximum number of lines in file
    uint32_t    trimmed_lines_;         // Number of lines after trimming
    uint32_t    line_count_;            // Lines in file
    time_t      last_timestamp_;        // Last timestamp printed

    void count_lines();
    const char *timestamp(const time_t *ts) const;

public:
    /**
     * @brief   Constructor
     * 
     * @param   filename        Name of log file
     * @param   max_lines       Line count where trimming will be done
     * @param   trimmed_lines   Lines to remain after trimming
     */
    FileLogger(const char *filename, uint32_t max_lines=2000, uint32_t trimmed_lines=1500);

    /**
     * @brief   Destructor
     */
    virtual ~FileLogger() {}

    /**
     * @brief   Print method with same function as printf
     */
    int print(const char *format, ...) override;

    /**
     * @brief   Trim file to trimmed_lines line count
     */
    bool trim_file();

    /**
     * @brief   Find the file position of a specified line
     * 
     * @param   line        Line number to locate
     * @param   from        Starting position to begin count
     * 
     * @return  File position for fseek to locate specified line
     */
    long find_line(int line, long from=0) const;

    /**
     * @brief   Find the file position a number of lines from the end
     * 
     * @details Actual line count may not be exactly as specified for
     *          large files
     * 
     * @param   lines       Number of lines before end
     * 
     * @return  File position for fseek to locate specified line
     */
    long find_tail(int lines) const;

    /**
     * @brief   Getter for current line count
     */
    uint32_t line_count() const { return line_count_; }

    /**
     * @brief   Getter for maximum line count
     */
    uint32_t max_line_count() const { return max_lines_; }

    /**
     * @brief   Getter for file size
     */
    uint32_t file_size() const;

    /**
     * @brief   Open log file for reading
     * 
     * @return  File handle (null if failed)
     */
    FILE *open() const { return fopen(filename_, "r"); }

    /**
     * @brief   Position file to line
     * 
     * @param   file    File handle returned by open
     * @param   pos     Position as returned by find_line or find_tail
     */
    void position(FILE *file, uint32_t pos) { if (file) fseek(file, pos, SEEK_SET); }

    /**
     * @brief   Read a line from the file
     * 
     * @param   file    File handle returned by open
     * @param   linebuf Line buffer
     * @param   bufsiz  Sie of line buffer
     * 
     * @return  true if line was read
     */
    bool read(FILE *file, char *linebuf, uint32_t bufsiz) { return file ? fgets(linebuf, bufsiz, file) != nullptr : false; }

    /**
     * @brief   Close file
     * 
     * @param   file    File handle returned by open
     */
    void close(FILE *file) { if (file) fclose(file); }

    /**
     * @brief   Call to initialize timestamps
     */
    void initialize_timestamps();
};

#endif
