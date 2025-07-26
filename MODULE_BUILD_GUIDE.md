# build guide
## install relation tools
- python 3.11 or higher
- conan2
- cmake 3.1 or higher

## conan2 prepare
install it an follow these step
- 1、conan profile detect --force
- 2、conan profile path default
- 3、modify cppstd to 20 in the default settings \
[settings]
compiler.cppstd=20
...
- 4、cd the source dir an execute \
conan install . --output-folder=build --build=missing --settings=build_type=Debug
- 5、cmake