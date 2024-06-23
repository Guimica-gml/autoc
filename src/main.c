#include "log.h"
#include "compile.h"
#include "config.h"
#include <assert.h>

/* Things to add/fix
* - If the length of a string such as target is longer than double the
*   previous length the formatting function may run out of memory
* - multiline commands for linking such as writing binary to another directory
* - Support for multiple directories
* - Support for precompiled headers
* - Custom linker commands
*/
int main(int argc, char** argv)
{
    struct config conf = {0};
    init_config(&conf);
    
    for (int i = 1; i < argc; i++)
    {
        char* arg = argv[i];

        if (*arg == '-') for (int j = 1; j < strlen(arg); j++)
        switch(arg[j])
        {
        case 'f':
            conf.force_compile = true;
            break;
        case 'r':
            conf.run_target = true;
            break;
        case 'c':
            conf.clear_bin = true;
            break;
        case 'h':
            flog(LOG_INFO, "Usage: %s <flags>", *argv);
            flog(LOG_INFO, "-f: force compile all modules");
            flog(LOG_INFO, "-c: clear binary path before compiling");
            flog(LOG_INFO, "-r: run target after compiling");
            flog(LOG_INFO, "-h: show this info");
            exit(0);
        }
        else
        {
            flog(LOG_INFO, "Unknown flag '%s'", arg);
            flog(LOG_INFO, "Usage: %s <flags>", *argv);
            flog(LOG_INFO, "-f: force compile all modules");
            flog(LOG_INFO, "-c: clear binary path before compiling");
            flog(LOG_INFO, "-r: run target after compiling");
            flog(LOG_INFO, "-h: show this info");
        }
    }

    time_t config_time = file_mod_time("./autoc.ini");

    const char** ls = get_directory_list(conf.src_dir);
    for (int i = 0; ls[i]; i++)
    {
        // flog(LOG_INFO, "Compiling file '%s'", ls[i]);
        const char* src = ls[i];
        char* bin = get_binary_from_source(src, conf.bin_dir); 
        
        time_t src_time = file_mod_time(src);
        time_t bin_time = file_mod_time(bin);

        if (conf.force_compile || MAX(src_time, config_time) > bin_time)
        if (compile(&conf, ls[i], conf.bin_dir))
        {
            flog(LOG_FATAL, "Failed to compile '%s'", ls[i]);
        }
    }
    if (link_to_target(&conf))
    {
        flog(LOG_FATAL, "Failed to link '%s'", conf.target);
    }
    if (conf.clear_bin)
    {
        char command[1024] = {0};
        assert(strlen("rm /*") + strlen(conf.bin_dir) < 1024);
        sprintf(command, "rm %s/*", conf.bin_dir);

        flog(LOG_INFO, "Running command '%s'", command); 
        if (system(command) == -1)
        {
            flog(LOG_FATAL, "Failed to run '%s': %s", command, strerror(errno));
        }
    }
    if (conf.run_target)
    {
        flog(LOG_INFO, "Running target '%s'", conf.target); 
        if (system(conf.target) == -1)
        {
            flog(LOG_FATAL, "Failed to run target '%s': %s", conf.target, strerror(errno));
        }
    }
    return 0;
}

