/* Generated native artwork, linked in ROM; never copy into work RAM. */
.section .rodata.potheads_cache,"a",%progbits
.balign 4
.global pot_film_cache_words
.type pot_film_cache_words,%object
pot_film_cache_words:
.incbin "build/potheads-opening-cache.bin"
.size pot_film_cache_words, .-pot_film_cache_words
