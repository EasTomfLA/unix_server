#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <zlib.h>
#include <sstream>
#include <iomanip>
// #include <openssl/md5.h>
#include <elf.h>

// template<typename ElfHeaderType /*Elf{32,64}_Ehdr*/,
// 	 typename ElfSectionHeaderType /*Elf{32,64}_Shdr*/,
// 	 typename ElfDynamicSectionEntryType /* Elf{32,64}_Dyn */>

std::vector<uint8_t> readCodeSection(const std::vector<uint8_t>& elfData) {
    // 解析 ELF 文件的头部
    const Elf64_Ehdr* elfHeader = reinterpret_cast<const Elf64_Ehdr*>(elfData.data());

    // 获取节表的位置和大小
    Elf64_Off sectionTableOffset = elfHeader->e_shoff;
    Elf64_Half sectionEntrySize = elfHeader->e_shentsize;
    Elf64_Half sectionEntryCount = elfHeader->e_shnum;

    // 找到 .text 节的起始地址和大小
    const Elf64_Shdr* sectionHeaderTable = reinterpret_cast<const Elf64_Shdr*>(elfData.data() + sectionTableOffset);
    const Elf64_Shdr* textSectionHeader = nullptr;

    for (int i = 0; i < sectionEntryCount; i++) {
        if (sectionHeaderTable[i].sh_type == SHT_PROGBITS && sectionHeaderTable[i].sh_flags == (SHF_ALLOC | SHF_EXECINSTR)) {
            textSectionHeader = &sectionHeaderTable[i];
            break;
        }
    }

    if (textSectionHeader == nullptr) {
        // 如果找不到 .text 节，返回空的代码块数据
        return {};
    }

    // 提取 .text 节的代码块数据
    Elf64_Off textSectionOffset = textSectionHeader->sh_offset;
    Elf64_Xword textSectionSize = textSectionHeader->sh_size;
    std::vector<uint8_t> codeData(elfData.begin() + textSectionOffset, elfData.begin() + textSectionOffset + textSectionSize);

    return codeData;
}

#define MAX_PATH_LENGTH 1024
typedef struct {
    unsigned long start;
    unsigned long end;
    char perms[5];
    unsigned long offset;
    int dev_major;
    int dev_minor;
    int inode;
    char path[MAX_PATH_LENGTH];
} ModuleInfo;

bool getProcessModulePaths(int pid, const char* moduleName, ModuleInfo& moduleInfo) {
    char mapsPath[MAX_PATH_LENGTH];
    snprintf(mapsPath, sizeof(mapsPath), "/proc/%d/maps", pid);

    FILE* mapsFile = fopen(mapsPath, "r");
    if (mapsFile == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", mapsPath);
        return false;
    }

    char line[MAX_PATH_LENGTH];
    while (fgets(line, sizeof(line), mapsFile) != NULL) {
        char modulePath[MAX_PATH_LENGTH];
        sscanf(line, "%*s %*s %*s %*s %*s %*s %s", modulePath);
        sscanf(line, "%lx-%lx %4s %lx %x:%x %d %[^\n]", &moduleInfo.start, &moduleInfo.end, moduleInfo.perms, &moduleInfo.offset, &moduleInfo.dev_major, &moduleInfo.dev_minor, &moduleInfo.inode, moduleInfo.path);
        if(strstr(line, moduleName) != NULL && strstr(moduleInfo.perms, "xp") != nullptr) {
            fclose(mapsFile);
            return true;
        }
    }

    printf("pid:%d find module %s failed\n", pid, moduleName);
    fclose(mapsFile);
    return false;
}

// 读取 ELF 模块内容
std::vector<uint8_t> readElfModule(const std::string& modulePath) {
    std::vector<uint8_t> moduleData;

    int fd = open(modulePath.c_str(), O_RDONLY);
    if (fd != -1) {
        struct stat st;
        if (fstat(fd, &st) == 0) {
            size_t size = st.st_size;
            moduleData.resize(size);
            void* addr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
            if (addr != MAP_FAILED) {
                memcpy(moduleData.data(), addr, size);
                munmap(addr, size);
            }
        }
        close(fd);
    }

    return moduleData;
}


std::string calculateCRC(const std::vector<uint8_t>& data) {
    uint32_t crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, data.data(), data.size());

    std::stringstream ss;
    ss << std::hex << std::setw(8) << std::setfill('0') << crc;

    return ss.str();
}

const int MAX_READ = 4096;
const int MAX_PROCESSNAME_INFILE = 4096;
int find_pid(char * process_name){
	DIR *d;
	struct dirent *de;
	ssize_t readcount;
	char tmp[256];
	char tmp_name[256];
	char pro_name[256];
	char buffer[MAX_READ];
	char name_buffer[256];
	int pid;
	int tmp_pid;
 
	int fd,fd2;

 
	if((d = opendir("/proc")) == 0)
		return -1;
 
	while((de = readdir(d)) != 0){
		if(isdigit(de->d_name[0])){
			pid = atoi(de->d_name);
			sprintf(tmp,"/proc/%d/stat",pid);
			sprintf(tmp_name,"/proc/%d/cmdline",pid);
 
			fd = open(tmp,O_RDONLY);
			if(fd == -1)
				return -1;
 
			readcount = read(fd, (void*)buffer, MAX_READ);
			if(readcount == -1)
				return -1;
			close(fd);
 
			fd2 = open(tmp_name,O_RDONLY);
			if(fd2 == -1)
				return -1;
 
			readcount = read(fd2,(void*)name_buffer, 256);
			if(readcount == -1)
				return -1;
			name_buffer[readcount] = '\0';
			close(fd2);
 
			if(name_buffer[0]){			//in default, the full process name is keep in /proc/PID/cmdline					
				if(!strcmp(name_buffer,process_name) || strstr(name_buffer,process_name) != NULL){
					//strcpy(full_proname,name_buffer);										
					return pid; 
				}	   			
			}
 
 
			sscanf(buffer,"%d%*[^(](%[^)]",&tmp_pid,pro_name);
			if(tmp_pid != pid)
				return -1;
			if(!strcmp(pro_name,process_name) || strstr(pro_name,process_name) != NULL){				
				return pid;
			}else{
				if(strlen(pro_name) == MAX_PROCESSNAME_INFILE && strlen(process_name) >= MAX_PROCESSNAME_INFILE)//the max length in stat/status of the [process name] value.
					if(strstr(process_name,pro_name) != NULL)						
						return pid;
			}
 
			
		}
	}
    return -1;
}

std::string calculateMemCRC(size_t pid, size_t start, size_t end) {
    size_t codeSize = end - start;
    std::vector<uint8_t> codeBuffer(codeSize);

    std::string memPath = "/proc/" + std::to_string(pid) + "/mem";
    FILE* memFile = fopen(memPath.c_str(), "r");
    if (memFile == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", memPath.c_str());
        return 0;
    }

    if (fseek(memFile, start, SEEK_SET) != 0) {
        fprintf(stderr, "Failed to seek to the module start address\n");
        fclose(memFile);
        return 0;
    }
    size_t readSize = fread(codeBuffer.data(), codeSize, 1, memFile);
    if (readSize != 1) {
        fprintf(stderr, "Failed to read module code, readSize:%ld \n", readSize);
        fclose(memFile);
        return 0;
    }

    fclose(memFile);
    std::string crcMem = calculateCRC(codeBuffer);
    return crcMem;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <process_name> <elf_module_name>" << std::endl;
        return 1;
    }

    std::string processName = argv[1];
    std::string elfModuleName = argv[2];

    fprintf(stdout, "input params processName:%s moduleName:%s\r\n", processName.c_str(), elfModuleName.c_str());

    int pid = find_pid(const_cast<char*>(processName.c_str()));
    if (pid == -1) {
        std::cerr << "can not find process name of pid:" << processName << std::endl;
        exit(-1);
    }
    std::cout << "found " << processName << " pid:" << pid << std::endl;
    
    ModuleInfo moduleInfo;
    if (!getProcessModulePaths(pid, elfModuleName.c_str(), moduleInfo)) {
        std::cerr << "find module failed! module name:" << elfModuleName.c_str() << std::endl;
        return 2;
    }

    // std::cout << "found match elf path:" << moduleInfo.path << std::endl;
    printf("found match elf\r\n%lx-%lx %4s %lx %x:%x %d %s\r\n", moduleInfo.start, moduleInfo.end, moduleInfo.perms, moduleInfo.offset, moduleInfo.dev_major, moduleInfo.dev_minor, moduleInfo.inode, moduleInfo.path);
    // std::string modulePath = moduleInfo.path;
    // std::vector<uint8_t> moduleData = readElfModule(modulePath);
    // moduleData = readCodeSection(moduleData);
    // std::cout << "moduleData read:" <<  moduleData.size() << std::endl;
    // std::string crc = calculateCRC(moduleData);
    // std::cout << "calculateCRC:" << crc << std::endl;

    std::string crcMem = calculateMemCRC(pid, moduleInfo.start, moduleInfo.end);
    std::cout << "calculateCRC mem:" << crcMem << std::endl;
    fprintf(stdout, "init code section crc monitor\n");


    while (true)
    {
        std::string crcMemRealTime = calculateMemCRC(pid, moduleInfo.start, moduleInfo.end);
        if (crcMemRealTime != crcMem)
        {
            fprintf(stdout, "code section changed! %s ==> %s\r\n", crcMem.c_str(), crcMemRealTime.c_str());
        }
        sleep(3);
    }
    
    return 0;
}