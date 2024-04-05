#include "Error.hpp"

std::string streamState(const std::fstream& file)
{
	if (file.rdstate() == std::ios_base::goodbit)
		return "No errors";
	std::string state;
	if (file.rdstate() & std::ios_base::badbit)
	  state += "Irrecoverable stream error; ";
	if (file.rdstate() & std::ios_base::failbit)
		state += "Non-fatal I/O error (e.g., nonexistent file, permission denied); ";
	if (file.rdstate() & std::ios_base::eofbit)
	    state += "End-of-File reached on input operation; ";
	return state;
}