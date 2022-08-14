#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <sstream>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <conio.h>

// @Zedritsch#8421

const char SPACE_PROOF[29] = " ()[]{}.,;:!?=+-*/%&|<>^~`\"'";

void refactor(const char *I_PATH, const char *O_PATH) {
	std::ifstream i_file(I_PATH);
	std::ofstream o_file(O_PATH);
	std::string old_line, new_line;
	unsigned long long length;
	char last_char, this_char, next_char, string_mode = '\0';
	bool comment;

	// Go trough Line by Line
	while (getline(i_file, old_line)) {
		if (old_line == "") continue;
		comment = false;
		new_line = "";
		this_char = ' ';
		next_char = old_line[0];
		length = old_line.length();

		// Go trough each Character
		for (unsigned long long i = 1; i < length; i++) {
			last_char = this_char;
			this_char = next_char;
			next_char = old_line[i];

			if (string_mode != '\0' && this_char != string_mode) {
				new_line.push_back(this_char);
				continue;
			}

			// Check if the Character is one of the Following:
			// ['\t', ' ', '"', '\'', '/', '<', '\\']
			// And if it matches the Conditions, skip it
			switch (this_char) {
				case '\t':
					break;
				case ' ':
					if (strchr(SPACE_PROOF, next_char) == NULL && strchr(SPACE_PROOF, last_char) == NULL) new_line.push_back(this_char);
					break;
				case '/':
					if (next_char == '/') {
						i = length;
						comment = true;
					}
					else if (next_char == '*') {
						this_char = old_line[i++];
						next_char = old_line[i++];
						while(this_char != '*' || next_char != '/') {
							this_char = next_char;
							next_char = old_line[i++];
						}
						i++;
						if (i < length) next_char = old_line[i];
						else comment = true;
					}
					else new_line.push_back(this_char);
					break;
				case '<':
					if (old_line.substr(i, 3) == "!--") {
						i += 3;
						while (old_line.substr(i, 3) != "-->") i++;
						i += 3;
						if (i < length) next_char = old_line[i];
						else comment = true;
					}
					else new_line.push_back(this_char);
					break;
				case '\\':
					new_line.push_back(this_char);
					new_line.push_back(next_char);
					i++;
					break;
				case '"':
				case '\'':
				case '`':
					string_mode = (string_mode == this_char)? '\0' : this_char;
				default:
					new_line.push_back(this_char);
			}
		}
		if (!comment) new_line.push_back(old_line[length - 1]);

		// Then write the Line to the Output File
		o_file << new_line;
	}

	i_file.close();
	o_file.close();
}

void searchTroughDir(const char DIR_NAME[]) {
	// Get the Files in the Output-Directory
	DIR *dirp = opendir(DIR_NAME);
	struct dirent *dp;
	readdir(dirp);
	readdir(dirp);
	std::string str;
	int pos;

	while ((dp = readdir(dirp)) != NULL) {
		str = dp->d_name;
		pos = str.find_last_of('.') +1;
		if(pos == 0) {
			searchTroughDir((std::string(DIR_NAME) + '\\' + dp->d_name).c_str());
		}
		else {
			str = str.substr(pos, str.length() - pos);
			if (str == "html" || str == "css" || str == "js") {
				std::cout << "Processing " << DIR_NAME << '\\' << dp->d_name << "...\n";
				// Refactor given File
				refactor(
					(std::string(DIR_NAME) + '\\' + dp->d_name).c_str(),
					("public" + std::string(DIR_NAME).substr(4) + '\\' + dp->d_name).c_str()
				);
				continue;
			}
		}
	}

	closedir(dirp);
}


int main(void) {
	system("xcopy dist public /t /e /Y");

	// Search for `.html`, `.css` & `.js` Files
	searchTroughDir("dist");
	std::cout << "Finished!\n";
}