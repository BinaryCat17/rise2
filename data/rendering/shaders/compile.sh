cd scene || exit
glslangValidator -V -S vert -o shader.vert.spv shader.vert
glslangValidator -V -S frag -o shader.frag.spv shader.frag
echo "Scene shaders compiled"
cd ..
cd gui || exit
glslangValidator -V -S vert -o shader.vert.spv shader.vert
glslangValidator -V -S frag -o shader.frag.spv shader.frag
echo "Gui shaders compiled"
cd ..
cd shadows || exit
glslangValidator -V -S vert -o shader.vert.spv shader.vert
glslangValidator -V -S frag -o shader.frag.spv shader.frag
glslangValidator -V -S geom -o shader.geom.spv shader.geom
echo "Shadows shaders compiled"
cd ..