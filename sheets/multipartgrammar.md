; Main multipart structure
multipart-body    = [preamble] 
                    initial-delimiter CRLF
                    body-part
                    *(encapsulation)
                    close-delimiter
                    [epilogue]

; Individual part encapsulation  
encapsulation     = delimiter CRLF body-part

; Boundary delimiters
initial-delimiter = "--" boundary
delimiter         = CRLF "--" boundary  
close-delimiter   = CRLF "--" boundary "--"

; Optional sections
preamble          = *OCTET                    ; Ignored by parsers
epilogue          = *OCTET                    ; Ignored by parsers

; Part structure
body-part         = *part-header blank-line part-body
part-header       = field-name ":" [field-value] CRLF
blank-line        = CRLF
part-body         = *OCTET                    ; Must not contain boundary

; Boundary definition (from Content-Type parameter)
boundary          = 1*70boundary-char
boundary-char     = DIGIT / ALPHA / "'" / "(" / ")" / "+" / "_" /
                    "," / "-" / "." / "/" / ":" / "=" / "?"