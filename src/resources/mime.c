#include "mime.h"

const char *mime_extensions[MIME_TYPES] = {
  ".aac", // AAC audio
  ".abw", // AbiWord document
  ".arc", // Archive document (multiple files embedded)
  ".avi", // AVI: Audio Video Interleave
  ".azw", // Amazon Kindle eBook format
  ".bin", // Any kind of binary data
  ".bmp", // Windows OS/2 Bitmap Graphics
  ".bz", // BZip archive
  ".bz2", // BZip2 archive
  ".csh", // C-Shell script
  ".css", // Cascading Style Sheets (CSS)
  ".csv", // Comma-separated values (CSV)
  ".doc", // Microsoft Word
  ".docx", // Microsoft Word (OpenXML)
  ".eot", // MS Embedded OpenType fonts
  ".epub", // Electronic publication (EPUB)
  ".gz", // GZip Compressed Archive
  ".gif", // Graphics Interchange Format (GIF)
  ".ico", // Icon format
  ".ics", // iCalendar format
  ".jar", // Java Archive (JAR)
  ".js", // JavaScript
  ".json", // JSON format
  ".jsonld", // JSON-LD format
  ".mjs", // JavaScript module
  ".mp3", // MP3 audio
  ".mpeg", // MPEG Video
  ".mpkg", // Apple Installer Package
  ".odp", // OpenDocument presentation document
  ".ods", // OpenDocument spreadsheet document
  ".odt", // OpenDocument text document
  ".oga", // OGG audio
  ".ogv", // OGG video
  ".ogx", // OGG
  ".otf", // OpenType font
  ".png", // Portable Network Graphics
  ".pdf", // Adobe Portable Document Format (PDF)
  ".php", // Hypertext Preprocessor (Personal Home Page)
  ".ppt", // Microsoft PowerPoint
  ".pptx", // Microsoft PowerPoint (OpenXML)
  ".rar", // RAR archive
  ".rtf", // Rich Text Format (RTF)
  ".sh", // Bourne shell script
  ".svg", // Scalable Vector Graphics (SVG)
  ".swf", // Small web format (SWF) or Adobe Flash document
  ".tar", // Tape Archive (TAR)
  ".ts", // MPEG transport stream
  ".ttf", // TrueType Font
  ".txt", // Text, (generally ASCII or ISO 8859-n)
  ".vsd", // Microsoft Visio
  ".wav", // Waveform Audio Format
  ".weba", // WEBM audio
  ".webm", // WEBM video
  ".webp", // WEBP image
  ".woff", // Web Open Font Format (WOFF)
  ".woff2", // Web Open Font Format (WOFF)
  ".xhtml", // XHTML
  ".xls", // Microsoft Excel
  ".xlsx", // Microsoft Excel (OpenXML)
  ".xul", // XUL
  ".zip", // ZIP archive
  ".7z" // 7-zip archive
  };

const char *mime_types[MIME_TYPES] = {
  "audio/aac", // AAC audio
  "application/x-abiword", // AbiWord document
  "application/x-freearc", // Archive document (multiple files embedded)
  "video/x-msvideo", // AVI: Audio Video Interleave
  "application/vnd.amazon.ebook", // Amazon Kindle eBook format
  "application/octet-stream", // Any kind of binary data
  "image/bmp", // Windows OS/2 Bitmap Graphics
  "application/x-bzip", // BZip archive
  "application/x-bzip2", // BZip2 archive
  "application/x-csh", // C-Shell script
  "text/css", // Cascading Style Sheets (CSS)
  "text/csv", // Comma-separated values (CSV)
  "application/msword", // Microsoft Word
  "application/vnd.openxmlformats-officedocument.wordprocessingml.document", // Microsoft Word (OpenXML)
  "application/vnd.ms-fontobject", // MS Embedded OpenType fonts
  "application/epub+zip", // Electronic publication (EPUB)
  "application/gzip", // GZip Compressed Archive
  "image/gif", // Graphics Interchange Format (GIF)
  "image/vnd.microsoft.icon", // Icon format
  "text/calendar", // iCalendar format
  "application/java-archive", // Java Archive (JAR)
  "text/javascript", // JavaScript
  "application/json", // JSON format
  "application/ld+json", // JSON-LD format
  "text/javascript", // JavaScript module
  "audio/mpeg", // MP3 audio
  "video/mpeg", // MPEG Video
  "application/vnd.apple.installer+xml", // Apple Installer Package
  "application/vnd.oasis.opendocument.presentation", // OpenDocument presentation document
  "application/vnd.oasis.opendocument.spreadsheet", // OpenDocument spreadsheet document
  "application/vnd.oasis.opendocument.text", // OpenDocument text document
  "audio/ogg", // OGG audio
  "video/ogg", // OGG video
  "application/ogg", // OGG
  "font/otf", // OpenType font
  "image/png", // Portable Network Graphics
  "application/pdf", // Adobe Portable Document Format (PDF)
  "appliction/php", // Hypertext Preprocessor (Personal Home Page)
  "application/vnd.ms-powerpoint", // Microsoft PowerPoint
  "application/vnd.openxmlformats-officedocument.presentationml.presentation", // Microsoft PowerPoint (OpenXML)
  "application/x-rar-compressed", // RAR archive
  "application/rtf", // Rich Text Format (RTF)
  "application/x-sh", // Bourne shell script
  "image/svg+xml", // Scalable Vector Graphics (SVG)
  "application/x-shockwave-flash", // Small web format (SWF) or Adobe Flash document
  "application/x-tar", // Tape Archive (TAR)
  "video/mp2t", // MPEG transport stream
  "font/ttf", // TrueType Font
  "text/plain", // Text, (generally ASCII or ISO 8859-n)
  "application/vnd.visio", // Microsoft Visio
  "audio/wav", // Waveform Audio Format
  "audio/webm", // WEBM audio
  "video/webm", // WEBM video
  "image/webp", // WEBP image
  "font/woff", // Web Open Font Format (WOFF)
  "font/woff2", // Web Open Font Format (WOFF)
  "application/xhtml+xml", // XHTML
  "application/vnd.ms-excel", // Microsoft Excel
  "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", // Microsoft Excel (OpenXML)
  "application/vnd.mozilla.xul+xml", // XUL
  "application/zip", // ZIP archive
  "application/x-7z-compressed" // 7-zip archive
};
