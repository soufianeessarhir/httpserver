# Top-level HTTP request structure
HTTP-request = request-line CRLF
               *( header-field CRLF )
               CRLF
               [ message-body ]
# Request line components
request-line = method SP request-target SP HTTP-version
# Method definition
method = token  # GET, POST, DELETE, 
# Request target (typically URI)
request-target = origin-form       ; most common
origin-form    = absolute-path [ "?" query ]
# HTTP version
HTTP-version = HTTP-name "/" DIGIT "." DIGIT 
HTTP-name    = "HTTP"
# Header fields
header-field   = field-name ":" OWS field-value OWS
field-name     = token
field-value    = *( field-content / obs-fold )
field-content  = field-vchar [ 1*( SP / HTAB ) field-vchar ]
field-vchar    = VCHAR / obs-text
obs-fold       = CRLF 1*( SP / HTAB )
# Message body (simplified)
message-body = *OCTET
# Basic rules
token       = 1*tchar
tchar       = "!" / "#" / "$" / "%" / "&" / "'" / "*"
            / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"
            / DIGIT / ALPHA
OWS         = *( SP / HTAB )  # optional whitespace
SP          = %x20  # space
HTAB        = %x09  # horizontal tab
CRLF        = CR LF # carriage return followed by line feed
CR          = %x0D  # carriage return 
LF          = %x0A  # line feed