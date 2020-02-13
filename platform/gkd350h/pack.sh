rm ./pokemini-opk/pokemini
cp ./pokemini ./pokemini-opk/pokemini
mksquashfs ./pokemini-opk pokemini.opk -all-root -noappend -no-exports -no-xattrs
