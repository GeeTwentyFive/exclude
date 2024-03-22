#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>



#define MAX_PATH 4096
#define MAX_ARGS 4







int SeperatePath(char* out_base_dir_path, char* out_filename, char* target) {

	char* target_split_address = NULL;
	for (int i = strlen(target); i > 0; i--) {

		if (target[i] == '/' || target[i] == '\\') {

			target_split_address = target+i; break;

		}

	}



	if (target_split_address == NULL) {

		char current_dir[MAX_PATH];
		if (getcwd(current_dir, MAX_PATH) == NULL) { puts("ERROR: getcwd() failed in SeperatePath()"); return -2; }

		strcpy(out_base_dir_path, current_dir);

		strcpy(out_filename, target);

	} else {

		strncpy(out_base_dir_path, target, (size_t)(target_split_address - target));

		strcpy(out_filename, target_split_address+1);

	}



	return 0;

}







void err_file(char* err_msg, char* file_name) {

	char* out = (char*) malloc(strlen(err_msg) + 2 + strlen(file_name) + 1);
	if (out == NULL) { puts("ERROR: malloc() failed in err_file()"); return; }

	strcpy(out, err_msg);
	strcat(out, " \"");
	strcat(out, file_name);
	strcat(out, "\"");

	puts(out);

}







char* ReadAll(long int* out_target_file_size, char* target_file_path) {

	FILE* target_file = fopen(target_file_path, "rb");
	if (target_file == NULL) { err_file("ERROR: Failed to open file:", target_file_path); return (char*)-1; }

	if (fseek(target_file, 0, SEEK_END) != 0) { err_file("ERROR: Failed to seek to end of file:", target_file_path); return (char*)-1; }

	long int target_file_size = ftell(target_file);
	if (target_file_size == -1L) { err_file("ERROR: Failed to tell on file:", target_file_path); return (char*)-1; }

	if (fseek(target_file, 0, SEEK_SET) != 0) { err_file("ERROR: Failed to seek to start of file:", target_file_path); return (char*)-1; }

	char* target_file_data = (char*) malloc(target_file_size+1);
	if (target_file_data == NULL) { puts("ERROR: target_file_data malloc() failed in ReadAll()"); return (char*)-1; }
	
	size_t read = fread(target_file_data, 1, target_file_size, target_file);
	if (read != target_file_size) { err_file("ERROR: Failed to read from file:", target_file_path); return (char*)-1; }

	target_file_data[target_file_size+1] = '\0';



	if (fclose(target_file) != 0) { err_file("ERROR: Failed to close file:", target_file_path); return (char*)-1; }

	*out_target_file_size = target_file_size;

	return target_file_data;

}







static char* include_pre;
static size_t include_pre_len;

static char* include_post;
static size_t include_post_len;

int Concatenate(char* target_file_path) {

	char start_dir[MAX_PATH];
	if (getcwd(start_dir, MAX_PATH) == NULL) { puts("ERROR: getcwd() failed in Concatenate()"); return -1; }



	char target_file_base_dir_path[MAX_PATH] = {0};
	char target_file_name[MAX_PATH] = {0};
	if (SeperatePath(target_file_base_dir_path, target_file_name, target_file_path) != 0) { puts("ERROR: SeperatePath() failed in Concatenate()"); return -1; }



	if (chdir(target_file_base_dir_path) != 0) { puts("ERROR: chdir(target_file_base_dir_path) failed in Concatenate()"); return -1; }



	long int target_file_size;
	char* target_file_data = ReadAll(&target_file_size, target_file_name);
	if (target_file_data == (char*)-1) { puts("ERROR: ReadAll() failed in Concatenate()"); return -1; }



	short include_file_path_len;
	char include_file_path[MAX_PATH];
	for (int i = 0; i < target_file_size; i++) {

		if (target_file_data[i] == include_pre[0]) {

			if (strncmp(target_file_data+i, include_pre, include_pre_len) == 0) {

				include_file_path_len = 0;
				for (char* j = target_file_data+i+include_pre_len; strncmp(j, include_post, include_post_len); j++) { include_file_path_len++; }

				strncpy(include_file_path, target_file_data+i+include_pre_len, include_file_path_len);

				int err = Concatenate(include_file_path);
				if (err != 0) { err_file("ERROR: Concatenate() recursive call failed on file:", include_file_path); return -1; }



				i += include_pre_len + include_file_path_len + include_post_len;

			}

		}

		putchar(target_file_data[i]);

	}



	free(target_file_data);



	if (chdir(start_dir) != 0) { puts("ERROR: chdir(start_dir) failed in Concatenate()"); return -1; }



	return 0;

}







int main(int argc, char* argv[]) {

	if (argc != MAX_ARGS-1 && argc != MAX_ARGS) { puts("USAGE: <TARGET_FILE> <INCLUDE_PRE> [INCLUDE_POST]"); return -1; }



	include_pre = argv[2];
	include_pre_len = strlen(argv[2]);

	if (argc != MAX_ARGS) {
	
		include_post = "\n";
		include_post_len = 1;
	
	} else {
	
		include_post = argv[3];
		include_post_len = strlen(argv[3]);
	
	}



	int err = Concatenate(argv[1]);
	if (err != 0) { puts("-- ^ ERROR(S) ^ --"); return -1; }



	return 0;

}

