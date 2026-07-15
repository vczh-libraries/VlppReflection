.PHONY: all clean pre-build
.DEFAULT_GOAL := all

CPP_COMPILE_OPTIONS=-I ../../../Import -DVCZH_DEBUG_METAONLY_REFLECTION
include $(VCPROOT)/vl/makefile-cpp

pre-build:
	if ! [ -d ./Bin ]; then mkdir ./Bin; fi
	if ! [ -d ./Obj ]; then mkdir ./Obj; fi
	if ! [ -d ./Coverage ]; then mkdir ./Coverage; fi
	if ! [ -d ../../Output ]; then mkdir ../../Output; fi

clean:
	if [ -d ./Bin ]; then rm -r ./Bin; fi
	if [ -d ./Obj ]; then rm -r ./Obj; fi
	if [ -d ./Coverage ]; then rm -r ./Coverage; fi
	if [ -d ./../../Output ]; then rm -r ../../Output; fi

all:pre-build ./Bin/UnitTest

./Bin/UnitTest:./Obj/Vlpp.o ./Obj/Vlpp.Linux.o ./Obj/VlppOS.o ./Obj/VlppOS.Linux.o ./Obj/VlppRegex.o ./Obj/DescriptableObject.o ./Obj/DescriptableValue.o ./Obj/DescriptableInterfaces.o ./Obj/DescriptableInterfaces_Log.o ./Obj/DescriptableInterfaces_Metaonly.o ./Obj/DescriptableValue_Comparison.o ./Obj/Metadata.o ./Obj/PredefinedTypes.o ./Obj/TypedValueSerializerProvider.o ./Obj/Reflection.o ./Obj/TestPredefinedTypes.o ./Obj/LoadMetadata.o ./Obj/Main.o
	$(CPP_LINK)

./Obj/Vlpp.o: ../../../Import/Vlpp.cpp
	$(CPP_COMPILE)

./Obj/Vlpp.Linux.o: ../../../Import/Vlpp.Linux.cpp
	$(CPP_COMPILE)

./Obj/VlppOS.o: ../../../Import/VlppOS.cpp
	$(CPP_COMPILE)

./Obj/VlppOS.Linux.o: ../../../Import/VlppOS.Linux.cpp
	$(CPP_COMPILE)

./Obj/VlppRegex.o: ../../../Import/VlppRegex.cpp
	$(CPP_COMPILE)

./Obj/DescriptableObject.o: ../../../Source/Reflection/DescriptableObject.cpp
	$(CPP_COMPILE)

./Obj/DescriptableValue.o: ../../../Source/Reflection/DescriptableValue.cpp
	$(CPP_COMPILE)

./Obj/DescriptableInterfaces.o: ../../../Source/Reflection/DescriptableInterfaces.cpp
	$(CPP_COMPILE)

./Obj/DescriptableInterfaces_Log.o: ../../../Source/Reflection/DescriptableInterfaces_Log.cpp
	$(CPP_COMPILE)

./Obj/DescriptableInterfaces_Metaonly.o: ../../../Source/Reflection/DescriptableInterfaces_Metaonly.cpp
	$(CPP_COMPILE)

./Obj/DescriptableValue_Comparison.o: ../../../Source/Reflection/DescriptableValue_Comparison.cpp
	$(CPP_COMPILE)

./Obj/Metadata.o: ../../../Source/Reflection/Metadata/Metadata.cpp
	$(CPP_COMPILE)

./Obj/PredefinedTypes.o: ../../../Source/Reflection/Predefined/PredefinedTypes.cpp
	$(CPP_COMPILE)

./Obj/TypedValueSerializerProvider.o: ../../../Source/Reflection/Predefined/TypedValueSerializerProvider.cpp
	$(CPP_COMPILE)

./Obj/Reflection.o: ../../../Source/Reflection/Reflection/Reflection.cpp
	$(CPP_COMPILE)

./Obj/TestPredefinedTypes.o: ../../Source/TestPredefinedTypes.cpp
	$(CPP_COMPILE)

./Obj/LoadMetadata.o: ../../UnitTest/Metadata_Test/LoadMetadata.cpp
	$(CPP_COMPILE)

./Obj/Main.o: ../Main.cpp
	$(CPP_COMPILE)
