# sw-fec-codec
Open-source sliding window FEC codec (IETF hackathon)

Main goal is to develop an open-source C-language codec for a sliding window FEC code. These codes can boost performance of content delivery protocols in harsh environments where packet losses can be frequent, while keeping the FEC-related added latency low.

This development is done in the context of the "Coding for Efficient Network Communications" IRTF Research Group (NWCRG, ​https://datatracker.ietf.org/rg/nwcrg) and IETF hackathon (​https://trac.ietf.org/trac/ietf/meeting/wiki/103hackathon).

This work has strong relationships with the Generic API I-D (​https://datatracker.ietf.org/doc/draft-roca-nwcrg-generic-fec-api/) and RLC codes (​https://datatracker.ietf.org/doc/draft-ietf-tsvwg-rlc-fec-scheme/) as examples of sliding window codes. Possible applications to QUIC (​https://datatracker.ietf.org/doc/draft-swett-nwcrg-coding-for-quic/ and ​https://datatracker.ietf.org/doc/draft-roca-nwcrg-rlc-fec-scheme-for-quic/), among others, are expected. 
