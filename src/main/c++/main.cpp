
#ifdef __linux__ 
    #include <arpa/inet.h>
    #include <unistd.h>
#elif _WIN32
    #include <winsock2.h>
#endif
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <archive.h>
#include <archive_entry.h>
#include "httprequest.h"

std::ofstream log;

void print(const std::string& message);

std::string& getJavaUrl();

std::string& getJavaVersionUrl();

bool isJavaExists();

bool isFileExists (const std::string& name);

void downloadFile(const std::string& fileName, const std::string& url);

int compareFiles(const std::string& file1, const std::string file2);

void runApp();

void warn(const char *f, const char *m);

void fail(const char *f, const char *m, int r);

static void	extract(const char *filename, int do_extract, int flags);

static int	copy_data(struct archive *, struct archive *);

static int verbose = 0;

int main () {
    log.open("retro-player.log", std::ios::out | std::ios::trunc );
    print("Checking java availability");
    if(!javaExists()) {
	print("Play50hz has its own java virtual machine, different from the one you mave have already installed manually.");
        print("Play50hz java specific version not found, downloading it..");
        downloadFile("java.tar.gz", getJavaUrl()); 
	print("Java download complete.");   
	print("Unpacking java.tar.gz...");    
	extract( "java.tar.gz", 1, 0);
	print("Unpack java.tar.gz complete.");
    } else {
        print("Java found, checking version...");    
        downloadFile("expected-release", getJavaVersionUrl());  
        if(!compareFiles("java/release", "expected-release")) {
	    print("Play50hz java version not matching, downloading the latest one..."); 
            downloadFile("java.tar.gz", getJavaUrl()); 
            print("Java download complete.");
            print("Unpacking java.tar.gz..."); 
            extract( "java.tar.gz", 1, 0);
            print("Unpack java.tar.gz complete.");    
        } else { 
	    print("Java version is correct.");	
	}
    }
    print("Downloading latest version of the application...");
    downloadFile("play50hz-player.jar", "http://files.yildiz-games.be/play50hz/launcher/player-launcher.jar");
    print("Download latest version of the launcher complete.");	
    print("Starting Play50hz...");	
    runApp();
    
    return 0;
}

#ifdef __linux__ 
    std::string workingdir() {
        return get_current_dir_name();
    }

    void runApp() {
        std::string cmd = "\"" + workingdir() +  "/java/bin/java" + "\"" + " -jar play50hz-player.jar";
        system(cmd.c_str());
    } 
#elif _WIN32
    std::string workingdir() {
        char buf[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, buf);
        return std::string(buf);
    }

    void runApp() {
        std::string cmd = "\"" + workingdir() +  "/java/bin/java.exe" + "\"" + " -jar play50hz-player.jar";
        system(cmd.c_str());
    }
#endif


static size_t writeData(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}
	
void print(const std::string& message) {
    log << message << std::endl;
    std::cout << message << std::endl;	
}
	
std::string& getJavaUrl() {
    #ifdef __linux__
	return "http://files.yildiz-games.be/java_jre_linux64.tar.gz";
    #elif _WIN32
	return "http://files.yildiz-games.be/java_jre_win64.tar.gz";
    #endif
}

std::string& getJavaVersionUrl() {
    #ifdef __linux__ 
        return "http://files.yildiz-games.be/release_linux64";  
    #elif _WIN32
        return "http://files.yildiz-games.be/release";  
    #endif
}

void downloadFile(const std::string& fileName, const std::string& url) {
    print("Started the download, please wait."); 
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
	
inline bool isJavaExists() {
    #ifdef __linux__ 
        return isFileExists("java/bin/java");
    #elif _WIN32
        return isFileExists("java/bin/java.exe");
    #endif
    return false;    
}

static void
extract(const char *filename, int do_extract, int flags)
{
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
		fail("archive_read_open_filename()",
		    archive_error_string(a), r);
	for (;;) {
		r = archive_read_next_header(a, &entry);
		if (r == ARCHIVE_EOF)
			break;
		if (r != ARCHIVE_OK)
			fail("archive_read_next_header()",
			    archive_error_string(a), 1);
		if (verbose && do_extract)
			log << "x ";
		if (verbose || !do_extract)
			log << archive_entry_pathname(entry);
		if (do_extract) {
			r = archive_write_header(ext, entry);
			if (r != ARCHIVE_OK)
				warn("archive_write_header()",
				    archive_error_string(ext));
			else {
				copy_data(a, ext);
				r = archive_write_finish_entry(ext);
				if (r != ARCHIVE_OK)
					fail("archive_write_finish_entry()",
					    archive_error_string(ext), 1);
			}

		}
		if (verbose || !do_extract)
			log << std::endl;
	}
	archive_read_close(a);
	archive_read_free(a);
	
	archive_write_close(ext);
  	archive_write_free(ext);
}

static int
copy_data(struct archive *ar, struct archive *aw)
{
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
			warn("archive_write_data_block()",
			    archive_error_string(aw));
			return (r);
		}
	}
}

void warn(const char *f, const char *m)
{
	log << f << ":" << m << std::endl;
        std::cout << f << ":" << m << std::endl;
}

void fail(const char *f, const char *m, int r)
{
	warn(f, m);
	exit(r);
}

int compareFiles(const std::string& file1, const std::string file2) {
    std::fstream f1, f2;
    char name[20], c1, c2;
    int flag=3;

    f1.open(file1, std::ios::in);
    f2.open(file2, std::ios::in);

    while(1){
        c1=f1.get();
        c2=f2.get();
        if(c1!=c2){
            flag=0;
            break;
        }
        if((c1==EOF)||(c2==EOF))
            break;
    }
    f1.close();
    f2.close();
    return flag;
}
