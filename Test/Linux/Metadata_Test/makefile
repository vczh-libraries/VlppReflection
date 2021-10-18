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

./Bin/UnitTest:./Obj/Vlpp.o ./Obj/VlppOS.o ./Obj/VlppRegex.o ./Obj/GuiTypeDescriptor.o ./Obj/GuiTypeDescriptorBuilder.o ./Obj/GuiTypeDescriptorPredefined.o ./Obj/GuiTypeDescriptorReflection.o ./Obj/GuiTypeDescriptor_Log.o ./Obj/GuiTypeDescriptor_Metaonly.o ./Obj/TestMetadata.o ./Obj/TestPredefinedTypes.o ./Obj/LoadMetadata.o ./Obj/Main.o
	$(CPP_LINK)

./Obj/Vlpp.o: ../../../Import/Vlpp.cpp ../../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/VlppOS.o: ../../../Import/VlppOS.cpp ../../../Import/VlppOS.h ../../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/VlppRegex.o: ../../../Import/VlppRegex.cpp ../../../Import/VlppRegex.h ../../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/GuiTypeDescriptor.o: ../../../Source/Reflection/GuiTypeDescriptor.cpp ../../../Source/Reflection/GuiTypeDescriptorReflection.h ../../../Source/Reflection/GuiTypeDescriptorMacros.h ../../../Source/Reflection/GuiTypeDescriptorBuilder.h ../../../Source/Reflection/GuiTypeDescriptorPredefined.h ../../../Source/Reflection/GuiTypeDescriptor.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Source/Reflection/GuiTypeDescriptorBuilder_Container.h ../../../Source/Reflection/GuiTypeDescriptorBuilder_Function.h ../../../Source/Reflection/GuiTypeDescriptorBuilder_Struct.h
	$(CPP_COMPILE)

./Obj/GuiTypeDescriptorBuilder.o: ../../../Source/Reflection/GuiTypeDescriptorBuilder.cpp ../../../Source/Reflection/GuiTypeDescriptorBuilder.h ../../../Source/Reflection/GuiTypeDescriptorPredefined.h ../../../Source/Reflection/GuiTypeDescriptor.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/GuiTypeDescriptorPredefined.o: ../../../Source/Reflection/GuiTypeDescriptorPredefined.cpp ../../../Source/Reflection/GuiTypeDescriptorMacros.h ../../../Source/Reflection/GuiTypeDescriptorBuilder.h ../../../Source/Reflection/GuiTypeDescriptorPredefined.h ../../../Source/Reflection/GuiTypeDescriptor.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Source/Reflection/GuiTypeDescriptorBuilder_Container.h ../../../Source/Reflection/GuiTypeDescriptorBuilder_Function.h ../../../Source/Reflection/GuiTypeDescriptorBuilder_Struct.h ../../../Source/Reflection/GuiTypeDescriptorReflection.h
	$(CPP_COMPILE)

./Obj/GuiTypeDescriptorReflection.o: ../../../Source/Reflection/GuiTypeDescriptorReflection.cpp ../../../Source/Reflection/GuiTypeDescriptorReflection.h ../../../Source/Reflection/GuiTypeDescriptorMacros.h ../../../Source/Reflection/GuiTypeDescriptorBuilder.h ../../../Source/Reflection/GuiTypeDescriptorPredefined.h ../../../Source/Reflection/GuiTypeDescriptor.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Source/Reflection/GuiTypeDescriptorBuilder_Container.h ../../../Source/Reflection/GuiTypeDescriptorBuilder_Function.h ../../../Source/Reflection/GuiTypeDescriptorBuilder_Struct.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/GuiTypeDescriptor_Log.o: ../../../Source/Reflection/GuiTypeDescriptor_Log.cpp ../../../Source/Reflection/GuiTypeDescriptorReflection.h ../../../Source/Reflection/GuiTypeDescriptorMacros.h ../../../Source/Reflection/GuiTypeDescriptorBuilder.h ../../../Source/Reflection/GuiTypeDescriptorPredefined.h ../../../Source/Reflection/GuiTypeDescriptor.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Source/Reflection/GuiTypeDescriptorBuilder_Container.h ../../../Source/Reflection/GuiTypeDescriptorBuilder_Function.h ../../../Source/Reflection/GuiTypeDescriptorBuilder_Struct.h
	$(CPP_COMPILE)

./Obj/GuiTypeDescriptor_Metaonly.o: ../../../Source/Reflection/GuiTypeDescriptor_Metaonly.cpp ../../../Source/Reflection/GuiTypeDescriptorReflection.h ../../../Source/Reflection/GuiTypeDescriptorMacros.h ../../../Source/Reflection/GuiTypeDescriptorBuilder.h ../../../Source/Reflection/GuiTypeDescriptorPredefined.h ../../../Source/Reflection/GuiTypeDescriptor.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Source/Reflection/GuiTypeDescriptorBuilder_Container.h ../../../Source/Reflection/GuiTypeDescriptorBuilder_Function.h ../../../Source/Reflection/GuiTypeDescriptorBuilder_Struct.h
	$(CPP_COMPILE)

./Obj/TestMetadata.o: ../../Source/TestMetadata.cpp ../../Source/../../Source/Reflection/GuiTypeDescriptorReflection.h ../../Source/../../Source/Reflection/GuiTypeDescriptorMacros.h ../../Source/../../Source/Reflection/GuiTypeDescriptorBuilder.h ../../Source/../../Source/Reflection/GuiTypeDescriptorPredefined.h ../../Source/../../Source/Reflection/GuiTypeDescriptor.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../Source/../../Source/Reflection/GuiTypeDescriptorBuilder_Container.h ../../Source/../../Source/Reflection/GuiTypeDescriptorBuilder_Function.h ../../Source/../../Source/Reflection/GuiTypeDescriptorBuilder_Struct.h
	$(CPP_COMPILE)

./Obj/TestPredefinedTypes.o: ../../Source/TestPredefinedTypes.cpp ../../Source/../../Source/Reflection/GuiTypeDescriptorReflection.h ../../Source/../../Source/Reflection/GuiTypeDescriptorMacros.h ../../Source/../../Source/Reflection/GuiTypeDescriptorBuilder.h ../../Source/../../Source/Reflection/GuiTypeDescriptorPredefined.h ../../Source/../../Source/Reflection/GuiTypeDescriptor.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../Source/../../Source/Reflection/GuiTypeDescriptorBuilder_Container.h ../../Source/../../Source/Reflection/GuiTypeDescriptorBuilder_Function.h ../../Source/../../Source/Reflection/GuiTypeDescriptorBuilder_Struct.h
	$(CPP_COMPILE)

./Obj/LoadMetadata.o: ../../UnitTest/Metadata_Test/LoadMetadata.cpp ../../UnitTest/Metadata_Test/../../../Source/Reflection/GuiTypeDescriptorReflection.h ../../UnitTest/Metadata_Test/../../../Source/Reflection/GuiTypeDescriptorMacros.h ../../UnitTest/Metadata_Test/../../../Source/Reflection/GuiTypeDescriptorBuilder.h ../../UnitTest/Metadata_Test/../../../Source/Reflection/GuiTypeDescriptorPredefined.h ../../UnitTest/Metadata_Test/../../../Source/Reflection/GuiTypeDescriptor.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../UnitTest/Metadata_Test/../../../Source/Reflection/GuiTypeDescriptorBuilder_Container.h ../../UnitTest/Metadata_Test/../../../Source/Reflection/GuiTypeDescriptorBuilder_Function.h ../../UnitTest/Metadata_Test/../../../Source/Reflection/GuiTypeDescriptorBuilder_Struct.h
	$(CPP_COMPILE)

./Obj/Main.o: ../Main.cpp ../../../Import/VlppOS.h ../../../Import/Vlpp.h
	$(CPP_COMPILE)
