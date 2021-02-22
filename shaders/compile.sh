cd diffuse || exit
glslangValidator -V -S vert -o shader.vert.spv shader.vert
glslangValidator -V -S frag -o shader.frag.spv shader.frag
echo "Diffuse shaders compiled"
cd ..
cd gui || exit
glslangValidator -V -S vert -o shader.vert.spv shader.vert
glslangValidator -V -S frag -o shader.frag.spv shader.frag
echo "Gui shaders compiled"
cd ..