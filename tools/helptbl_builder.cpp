/* Build pages/helptbl from pages/help_src/MANIFEST.
 * Skips rewrite when content stamp matches (FNV-1a over manifest + fragments).
 *
 * Usage:
 *   helptbl_builder <manifest> <src_dir> <out_helptbl> <stamp_file>
 */
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr std::uint64_t kFnvOffset = 14695981039346656037ULL;
constexpr std::uint64_t kFnvPrime = 1099511628211ULL;

void fnv1a_update(std::uint64_t& h, const char* data, std::size_t n) {
	for(std::size_t i = 0; i < n; ++i) {
		h ^= static_cast<unsigned char>(data[i]);
		h *= kFnvPrime;
	}
}

void fnv1a_update(std::uint64_t& h, const std::string& s) {
	fnv1a_update(h, s.data(), s.size());
}

std::string to_hex(std::uint64_t v) {
	static const char* digits = "0123456789abcdef";
	std::string out(16, '0');
	for(int i = 15; i >= 0; --i) {
		out[static_cast<std::size_t>(i)] = digits[v & 0xf];
		v >>= 4;
	}
	return out;
}

std::string read_file(const std::string& path) {
	std::ifstream in(path, std::ios::binary);
	if(!in) {
		return {};
	}
	std::ostringstream ss;
	ss << in.rdbuf();
	return ss.str();
}

bool write_file(const std::string& path, const std::string& data) {
	std::ofstream out(path, std::ios::binary | std::ios::trunc);
	if(!out) {
		return false;
	}
	out.write(data.data(), static_cast<std::streamsize>(data.size()));
	return static_cast<bool>(out);
}

std::string normalize_newlines(std::string text) {
	std::string out;
	out.reserve(text.size());
	for(std::size_t i = 0; i < text.size(); ++i) {
		if(text[i] == '\r') {
			if(i + 1 < text.size() && text[i + 1] == '\n') {
				++i;
			}
			out.push_back('\n');
		}
		else {
			out.push_back(text[i]);
		}
	}
	return out;
}

std::string rstrip_newlines_spaces(std::string text) {
	while(!text.empty()) {
		const char c = text.back();
		if(c == '\n' || c == '\r' || c == ' ' || c == '\t') {
			text.pop_back();
		}
		else {
			break;
		}
	}
	return text;
}

/* Decode UTF-8 to code points, emit latin-1 ('?' if > 255); LF -> CRLF. */
std::string to_helptbl_bytes(const std::string& body_utf8_lf) {
	std::string out;
	out.reserve(body_utf8_lf.size() + body_utf8_lf.size() / 8);
	const auto* p = reinterpret_cast<const unsigned char*>(body_utf8_lf.data());
	const auto* end = p + body_utf8_lf.size();
	while(p < end) {
		std::uint32_t cp = 0;
		const unsigned char c = *p++;
		if(c < 0x80) {
			cp = c;
		}
		else if((c & 0xE0) == 0xC0 && p < end) {
			cp = (c & 0x1Fu) << 6;
			cp |= (*p++ & 0x3Fu);
		}
		else if((c & 0xF0) == 0xE0 && p + 1 < end) {
			cp = (c & 0x0Fu) << 12;
			cp |= (*p++ & 0x3Fu) << 6;
			cp |= (*p++ & 0x3Fu);
		}
		else if((c & 0xF8) == 0xF0 && p + 2 < end) {
			cp = (c & 0x07u) << 18;
			cp |= (*p++ & 0x3Fu) << 12;
			cp |= (*p++ & 0x3Fu) << 6;
			cp |= (*p++ & 0x3Fu);
		}
		else {
			cp = static_cast<std::uint32_t>('?');
		}

		if(cp == '\n') {
			out.push_back('\r');
			out.push_back('\n');
		}
		else if(cp <= 0xFF) {
			out.push_back(static_cast<char>(cp));
		}
		else {
			out.push_back('?');
		}
	}
	return out;
}

std::vector<std::string> read_manifest_names(const std::string& manifest_text) {
	std::vector<std::string> names;
	std::istringstream iss(manifest_text);
	std::string line;
	while(std::getline(iss, line)) {
		if(!line.empty() && line.back() == '\r') {
			line.pop_back();
		}
		/* trim */
		std::size_t a = 0;
		while(a < line.size() && (line[a] == ' ' || line[a] == '\t')) {
			++a;
		}
		std::size_t b = line.size();
		while(b > a && (line[b - 1] == ' ' || line[b - 1] == '\t')) {
			--b;
		}
		line = line.substr(a, b - a);
		if(line.empty() || line[0] == '#') {
			continue;
		}
		names.push_back(line);
	}
	return names;
}

} // namespace

int main(int argc, char** argv) {
	if(argc != 5) {
		std::cerr << "Usage: " << (argc > 0 ? argv[0] : "helptbl_builder")
				  << " <manifest> <src_dir> <out_helptbl> <stamp_file>\n";
		return 2;
	}

	const std::string manifest_path = argv[1];
	const std::string src_dir = argv[2];
	const std::string out_path = argv[3];
	const std::string stamp_path = argv[4];

	const std::string manifest_text = read_file(manifest_path);
	if(manifest_text.empty() && !std::ifstream(manifest_path)) {
		std::cerr << "helptbl_builder: missing manifest: " << manifest_path << "\n";
		return 1;
	}

	const auto names = read_manifest_names(manifest_text);
	std::uint64_t hash = kFnvOffset;
	fnv1a_update(hash, manifest_text);

	std::string body;
	for(const auto& name : names) {
		const std::string path = src_dir + "/" + name;
		std::string text = read_file(path);
		if(text.empty() && !std::ifstream(path)) {
			std::cerr << "helptbl_builder: missing file: " << path << "\n";
			return 1;
		}
		fnv1a_update(hash, name);
		fnv1a_update(hash, "\n", 1);
		fnv1a_update(hash, text);

		text = normalize_newlines(std::move(text));
		text = rstrip_newlines_spaces(std::move(text));
		body += text;
		body += "\n\n";
	}

	body = rstrip_newlines_spaces(std::move(body));
	body += "\n\n#~\n";

	const std::string stamp = to_hex(hash) + "\n";
	const std::string old_stamp = read_file(stamp_path);
	const bool out_exists = static_cast<bool>(std::ifstream(out_path));
	if(out_exists && old_stamp == stamp) {
		std::cout << "helptbl up-to-date (" << stamp_path << ")\n";
		return 0;
	}

	const std::string bytes = to_helptbl_bytes(body);
	if(!write_file(out_path, bytes)) {
		std::cerr << "helptbl_builder: cannot write " << out_path << "\n";
		return 1;
	}
	if(!write_file(stamp_path, stamp)) {
		std::cerr << "helptbl_builder: cannot write " << stamp_path << "\n";
		return 1;
	}

	std::cout << "Wrote " << out_path << " (" << bytes.size()
			  << " bytes), stamp " << stamp.substr(0, 16) << "…\n";
	return 0;
}
