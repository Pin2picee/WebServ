#ifndef UTILS_HPP
# define UTILS_HPP

# include "ConfigParser.hpp"

std::string	strip_semicolon(const std::string &s);
void		init_default_errors(ServerConf &conf);
inline void makeResponse(Response &res, int status, const std::string &body, const std::string &content_type = MIME_TEXT_PLAIN)
{
	res.status_code = status;
	res.body = body;
	res.content_type = content_type;
}

//=== MIME (Multipurpose Internet Mail Extensions) content types ===//
// Text types
# define MIME_TEXT_PLAIN       "text/plain"
# define MIME_TEXT_HTML        "text/html"
# define MIME_TEXT_CSS         "text/css"
# define MIME_TEXT_JAVASCRIPT  "application/javascript"

// Data / JSON
# define MIME_APPLICATION_JSON "application/json"
# define MIME_APPLICATION_XML  "application/xml"

// Images
# define MIME_IMAGE_PNG        "image/png"
# define MIME_IMAGE_JPEG       "image/jpeg"
# define MIME_IMAGE_GIF        "image/gif"
# define MIME_IMAGE_SVG        "image/svg+xml"

// Fonts
# define MIME_FONT_WOFF        "font/woff"
# define MIME_FONT_WOFF2       "font/woff2"
# define MIME_FONT_TTF         "font/ttf"

// Binary / Misc
# define MIME_APPLICATION_OCTET_STREAM "application/octet-stream"

// Text colors
# define RESET   "\033[0m"
# define RED     "\033[31m"
# define GREEN   "\033[32m"
# define YELLOW  "\033[33m"
# define MAGENTA "\033[35m"
# define CYAN    "\033[36m"

// Text styles (optionnel)
# define BOLD       "\033[1m"
# define UNDERLINE  "\033[4m"

#endif
