#include "httprequest.h"

#ifdef __aarch64__
    #include <arpa/inet.h>
    #include <unistd.h>

    const std::string javaUrl = std::string("http://files.yildiz-games.be/java_jre_linuxarm64.tar.gz");
    const std::string javaVersionUrl = std::string("http://files.yildiz-games.be/release_linuxarm64");
    const std::string javaFile = std::string("java/bin/java");
#elif __linux__
    #include <arpa/inet.h>
    #include <unistd.h>

    const std::string javaUrl = std::string("http://files.yildiz-games.be/java_jre_linux64.tar.gz");
    const std::string javaVersionUrl = std::string("http://files.yildiz-games.be/release_linux64");
    const std::string javaFile = std::string("java/bin/java");
#elif _WIN32
    const std::string javaUrl = std::string("http://files.yildiz-games.be/java_jre_win64.tar.gz");
    const std::string javaVersionUrl = std::string("http://files.yildiz-games.be/release");
    const std::string javaFile = std::string("java/bin/java.exe");
#endif

#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <archive.h>
#include <archive_entry.h>

const std::string launcherFile = std::string("play50hz-player.jar");

const std::string launcherVersionUrl = std::string("http://files.yildiz-games.be/launcher-release");

std::ofstream logger;

void print(const std::string& message);

bool isFileExists (const std::string& name);

void downloadFile(const std::string& fileName, const std::string& url);

int compareFiles(const std::string& file1, const std::string file2);

void getJava();

int runApp();

void warn(const char *f, const char *m);

void fail(const char *f, const char *m, int r);

static void extract(const char *filename, int do_extract, int flags);

static int copy_data(struct archive *, struct archive *);

static int verbose = 0;

int main () {
    logger.open("retro-player.log", std::ios::out | std::ios::trunc );
    print("Checking java availability");
    if(!isFileExists(javaFile)) {
        print("PXL has its own java virtual machine, different from the one you mave have already installed manually.");
        print("PXL java specific version not found, downloading it..");
        getJava();
    } else {
        print("Java found, checking version...");    
        downloadFile("expected-release", javaVersionUrl);  
        if(!compareFiles("java/release", "expected-release")) {
            print("PXL java version not matching, downloading the latest one..."); 
            getJava();
        } else { 
            print("Java version is correct.");	
        }
    }

    print("Checking PXL launcher availability");
    if(!isFileExists(launcherFile)) {
        print("PXL launcher not found, downloading it...");
        downloadFile(launcherFile, "http://files.yildiz-games.be/play50hz/launcher/player-launcher.jar");
    } else {
        print("PXL launcher found, checking version...");    
        downloadFile("expected-launcher-release", launcherVersionUrl);  
        if(!isFileExists("launcher-release") || !compareFiles("launcher-release", "expected-launcher-release")) {
            print("PXL launcher version not matching, downloading the latest one..."); 
            downloadFile(launcherFile, "http://files.yildiz-games.be/play50hz/launcher/player-launcher.jar");
        } else { 
            print("PXL launcher version is correct.");	
        }
    }

    print("Starting PXL...");	
    return runApp();
}

void getJava() {
    downloadFile("java.tar.gz", javaUrl); 
    print("Java download complete.");   
    print("Unpacking java.tar.gz...");    
    extract( "java.tar.gz", 1, 0);
    print("Unpack java.tar.gz complete.");	
}

std::string workingdir() {
#ifdef __linux__ 
    return get_current_dir_name();
#elif _WIN32
    char buf[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buf);
    return std::string(buf);
#endif
}

int runApp() {
    std::string cmd = "\"" + workingdir() +  "/" + javaFile + "\"" + " -jar --enable-native-access=ALL-UNNAMED,be.yildizgames.module.compression.sevenzip,be.yildizgames.module.controller.sdl,be.yildizgames.retro.player.emulator --add-opens javafx.graphics/javafx.scene.layout=be.yildizgames.module.window.javafx play50hz-player.jar";
    return system(cmd.c_str());
}
	
void print(const std::string& message) {
    logger << message << std::endl;
    std::cout << message << std::endl;	
}

void downloadFile(const std::string& fileName, const std::string& url) {
    try {
        http::Request request(url);
        const http::Response response = request.send("GET");
        std::ofstream outfile(fileName, std::ofstream::binary);
        outfile.write(reinterpret_cast<const char*>(response.body.data()), static_cast<std::streamsize>(response.body.size()));
    } catch (const std::exception& e) {
        std::cerr << "Request failed, error: " << e.what() << '\n';
    }
}

inline bool isFileExists (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}

static void extract(const char *filename, int do_extract, int flags) {
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int r;

    a = archive_read_new();
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_read_support_format_tar(a);

    if (filename != NULL && strcmp(filename, "-") == 0)
        filename = NULL;
    if ((r = archive_read_open_filename(a, filename, 10240)))
        fail("archive_read_open_filename()", archive_error_string(a), r);
    for (;;) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r != ARCHIVE_OK)
            fail("archive_read_next_header()", archive_error_string(a), 1);
        if (verbose && do_extract)
            logger << "x ";
        if (verbose || !do_extract)
            logger << archive_entry_pathname(entry);
        if (do_extract) {
            r = archive_write_header(ext, entry);
            if (r != ARCHIVE_OK)
                warn("archive_write_header()", archive_error_string(ext));
            else {
                copy_data(a, ext);
                r = archive_write_finish_entry(ext);
                if (r != ARCHIVE_OK)
                    fail("archive_write_finish_entry()", archive_error_string(ext), 1);
	    }
        }
        if (verbose || !do_extract)
            logger << std::endl;
    }
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
}

static int copy_data(struct archive *ar, struct archive *aw) {
    int r;
    const void *buff;
    size_t size;
#if ARCHIVE_VERSION_NUMBER >= 3000000
    int64_t offset;
#else
    off_t offset;
#endif

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r != ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK) {
            warn("archive_write_data_block()", archive_error_string(aw));
            return (r);
        }
    }
}

void warn(const char *f, const char *m) {
    logger << f << ":" << m << std::endl;
    std::cout << f << ":" << m << std::endl;
}

void fail(const char *f, const char *m, int r) {
    warn(f, m);
    exit(r);
}

int compareFiles(const std::string& file1, const std::string file2) {
    std::fstream f1, f2;
    char c1, c2;
    int flag = 3;

    f1.open(file1, std::ios::in);
    f2.open(file2, std::ios::in);
	int x = 0;
    while(1){
	x++;
        c1=f1.get();
        c2=f2.get();
        if(c1!=c2){
            flag=0;
            break;
        }
        if((c1==EOF)||(c2==EOF) || ((int)c1==255) || ((int)c2==255)) {
            break;
	}
    }
    f1.close();
    f2.close();
    return flag;
}
