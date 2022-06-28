/* -*- mode: cr; indent-width: 4; -*-
 * $Id: cmake.cr,v 1.2 2022/06/26 16:17:27 cvsuser Exp $
 * 'cmake' programming languages mode.
 *
 *
 */

#include "../grief.h"

#define MODENAME "cmake"

void
main()
{
    create_syntax(MODENAME);

    syntax_token(SYNT_COMMENT,      "/*", "*/");
    syntax_token(SYNT_COMMENT,      "//");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_CHARACTER,    "'");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_LINECONT,     "\\");
    syntax_token(SYNT_PREPROCESSOR, "#");
    syntax_token(SYNT_BRACKET,      "([{", ")]}");
    syntax_token(SYNT_WORD,         "0-9A-Z_a-z");
    syntax_token(SYNT_NUMERIC,      "-+.0-9_xa-fA-F");

    // cmake --help-command-list
    define_keywords(SYNK_PRIMARY,
	"add_compile_options,"+
	"add_custom_command,"+
	"add_custom_target,"+
	"add_definitions,"+
	"add_dependencies,"+
	"add_executable,"+
	"add_library,"+
	"add_subdirectory,"+
	"add_test,"+
	"aux_source_directory,"+
	"break,"+
	"build_command,"+
	"cmake_host_system_information,"+
	"cmake_minimum_required,"+
	"cmake_policy,"+
	"configure_file,"+
	"create_test_sourcelist,"+
	"define_property,"+
	"else,"+
	"elseif,"+
	"enable_language,"+
	"enable_testing,"+
	"endforeach,"+
	"endfunction,"+
	"endif,"+
	"endmacro,"+
	"endwhile,"+
	"execute_process,"+
	"export,"+
	"file,"+
	"find_file,"+
	"find_library,"+
	"find_package,"+
	"find_path,"+
	"find_program,"+
	"fltk_wrap_ui,"+
	"foreach,"+
	"function,"+
	"get_cmake_property,"+
	"get_directory_property,"+
	"get_filename_component,"+
	"get_property,"+
	"get_source_file_property,"+
	"get_target_property,"+
	"get_test_property,"+
	"if,"+
	"include,"+
	"include_directories,"+
	"include_external_msproject,"+
	"include_regular_expression,"+
	"install,"+
	"link_directories,"+
	"list,"+
	"load_cache,"+
	"load_command,"+
	"macro,"+
	"mark_as_advanced,"+
	"math,"+
	"message,"+
	"option,"+
	"project,"+
	"qt_wrap_cpp,"+
	"qt_wrap_ui,"+
	"remove_definitions,"+
	"return,"+
	"separate_arguments,"+
	"set,"+
	"set_directory_properties,"+
	"set_property,"+
	"set_source_files_properties,"+
	"set_target_properties,"+
	"set_tests_properties,"+
	"site_name,"+
	"source_group,"+
	"string,"+
	"target_compile_definitions,"+
	"target_compile_options,"+
	"target_include_directories,"+
	"target_link_libraries,"+
	"try_compile,"+
	"try_run,"+
	"unset,"+
	"variable_watch,"+
	"while"
	);

    // compatibility commands
    define_keywords(SYNK_PRIMARY,
	"build_name,"+
	"exec_program,"+
	"export_library_dependencies,"+
	"install_files,"+
	"install_programs,"+
	"install_targets,"+
	"link_libraries,"+
	"make_directory,"+
	"output_required_files,"+
	"remove,"+
	"subdir_depends,"+
	"subdirs,"+
	"use_mangled_mesa,"+
	"utility_source,"+
	"variable_requires,"+
	"write_file"
	);

    // cmake --help-property-list
    define_keywords(SYNK_CONSTANT,
	"ALLOW_DUPLICATE_CUSTOM_TARGETS,"+
	"AUTOMOC_TARGETS_FOLDER,"+
	"DEBUG_CONFIGURATIONS,"+
	"DISABLED_FEATURES,"+
	"ENABLED_FEATURES,"+
	"ENABLED_LANGUAGES,"+
	"FIND_LIBRARY_USE_LIB64_PATHS,"+
	"FIND_LIBRARY_USE_OPENBSD_VERSIONING,"+
	"GLOBAL_DEPENDS_DEBUG_MODE,"+
	"GLOBAL_DEPENDS_NO_CYCLES,"+
	"IN_TRY_COMPILE,"+
	"PACKAGES_FOUND,"+
	"PACKAGES_NOT_FOUND,"+
	"PREDEFINED_TARGETS_FOLDER,"+
	"REPORT_UNDEFINED_PROPERTIES,"+
	"RULE_LAUNCH_COMPILE,"+
	"RULE_LAUNCH_CUSTOM,"+
	"RULE_LAUNCH_LINK,"+
	"RULE_MESSAGES,"+
	"TARGET_ARCHIVES_MAY_BE_SHARED_LIBS,"+
	"TARGET_SUPPORTS_SHARED_LIBS,"+
	"USE_FOLDERS,"+
	"__CMAKE_DELETE_CACHE_CHANGE_VARS_,"+
	"ADDITIONAL_MAKE_CLEAN_FILES,"+
	"CACHE_VARIABLES,"+
	"CLEAN_NO_CUSTOM,"+
	"COMPILE_DEFINITIONS,"+
	    //"COMPILE_DEFINITIONS_*,"+
	"COMPILE_OPTIONS,"+
	"DEFINITIONS,"+
	"EXCLUDE_FROM_ALL,"+
	"IMPLICIT_DEPENDS_INCLUDE_TRANSFORM,"+
	"INCLUDE_DIRECTORIES,"+
	"INCLUDE_REGULAR_EXPRESSION,"+
	"INTERPROCEDURAL_OPTIMIZATION,"+
	    //"INTERPROCEDURAL_OPTIMIZATION_*,"+
	"LINK_DIRECTORIES,"+
	"LISTFILE_STACK,"+
	"MACROS,"+
	"PARENT_DIRECTORY,"+
	"RULE_LAUNCH_COMPILE,"+
	"RULE_LAUNCH_CUSTOM,"+
	"RULE_LAUNCH_LINK,"+
	"TEST_INCLUDE_FILE,"+
	"VARIABLES,"+
	    //"VS_GLOBAL_SECTION_POST_*
	    //"VS_GLOBAL_SECTION_PRE_*,"
	"ALIASED_TARGET,"+
	"ARCHIVE_OUTPUT_DIRECTORY,"+
	    //"ARCHIVE_OUTPUT_DIRECTORY_*,"+
	    //"ARCHIVE_OUTPUT_NAME,"+
	    //"ARCHIVE_OUTPUT_NAME_*,"+
	"AUTOMOC,"+
	"AUTOMOC_MOC_OPTIONS,"+
	"BUILD_WITH_INSTALL_RPATH,"+
	"BUNDLE,"+
	"BUNDLE_EXTENSION,"+
	"COMPATIBLE_INTERFACE_BOOL,"+
	"COMPATIBLE_INTERFACE_STRING,"+
	"COMPILE_DEFINITIONS,"+
	    //COMPILE_DEFINITIONS_*,"+
	"COMPILE_FLAGS,"+
	"COMPILE_OPTIONS,"+
	"DEBUG_POSTFIX,"+
	"DEFINE_SYMBOL,"+
	"ENABLE_EXPORTS,"+
	"EXCLUDE_FROM_ALL,"+
	"EXCLUDE_FROM_DEFAULT_BUILD,"+
	    //"EXCLUDE_FROM_DEFAULT_BUILD_*,"+
	"EXPORT_NAME,"+
	"EchoString,"+
	"FOLDER,"+
	"FRAMEWORK,"+
	"Fortran_FORMAT,"+
	"Fortran_MODULE_DIRECTORY,"+
	"GENERATOR_FILE_NAME,"+
	"GNUtoMS,"+
	"HAS_CXX,"+
	"IMPLICIT_DEPENDS_INCLUDE_TRANSFORM,"+
	"IMPORTED,"+
	"IMPORTED_CONFIGURATIONS,"+
	"IMPORTED_IMPLIB,"+
	    //IMPORTED_IMPLIB_*,"+
	"IMPORTED_LINK_DEPENDENT_LIBRARIES,"+
	    //IMPORTED_LINK_DEPENDENT_LIBRARIES_*,"+
	"IMPORTED_LINK_INTERFACE_LANGUAGES,"+
	    //IMPORTED_LINK_INTERFACE_LANGUAGES_*,"+
	"IMPORTED_LINK_INTERFACE_LIBRARIES,"+
	    //IMPORTED_LINK_INTERFACE_LIBRARIES_*,"+
	"IMPORTED_LINK_INTERFACE_MULTIPLICITY,"+
	    //IMPORTED_LINK_INTERFACE_MULTIPLICITY_*,"+
	"IMPORTED_LOCATION,"+
	    //IMPORTED_LOCATION_*,"+
	"IMPORTED_NO_SONAME,"+
	    //IMPORTED_NO_SONAME_*,"+
	"IMPORTED_SONAME,"+
	    //"IMPORTED_SONAME_*,"+
	"IMPORT_PREFIX,"+
	"IMPORT_SUFFIX,"+
	"INCLUDE_DIRECTORIES,"+
	"INSTALL_NAME_DIR,"+
	"INSTALL_RPATH,"+
	"INSTALL_RPATH_USE_LINK_PATH,"+
	"INTERFACE_COMPILE_DEFINITIONS,"+
	"INTERFACE_COMPILE_OPTIONS,"+
	"INTERFACE_INCLUDE_DIRECTORIES,"+
	"INTERFACE_LINK_LIBRARIES,"+
	"INTERFACE_POSITION_INDEPENDENT_CODE,"+
	"INTERFACE_SYSTEM_INCLUDE_DIRECTORIES,"+
	"INTERPROCEDURAL_OPTIMIZATION,"+
	    //"INTERPROCEDURAL_OPTIMIZATION_*,"+
	"LABELS,"+
	"LIBRARY_OUTPUT_DIRECTORY,"+
	    //"LIBRARY_OUTPUT_DIRECTORY_*,"+
	"LIBRARY_OUTPUT_NAME,"+
	    //"LIBRARY_OUTPUT_NAME_*,"+
	"LINKER_LANGUAGE,"+
	"LINK_DEPENDS,"+
	"LINK_DEPENDS_NO_SHARED,"+
	"LINK_FLAGS,"+
	    //"LINK_FLAGS_*,"+
	"LINK_INTERFACE_LIBRARIES,"+
	    //"LINK_INTERFACE_LIBRARIES_*,"+
	"LINK_INTERFACE_MULTIPLICITY,"+
	    //"LINK_INTERFACE_MULTIPLICITY_*,"+
	"LINK_LIBRARIES,"+
	"LINK_SEARCH_END_STATIC,"+
	"LINK_SEARCH_START_STATIC,"+
	"LOCATION,"+
	    //"LOCATION_*,"+
	"MACOSX_BUNDLE,"+
	"MACOSX_BUNDLE_INFO_PLIST,"+
	"MACOSX_FRAMEWORK_INFO_PLIST,"+
	"MACOSX_RPATH,"+
	    //"MAP_IMPORTED_CONFIG_*,"+
	"NAME,"+
	"NO_SONAME,"+
	"OSX_ARCHITECTURES,"+
	    //"OSX_ARCHITECTURES_*,"+
	"OUTPUT_NAME,"+
	    //"OUTPUT_NAME_*,"+
	"PDB_NAME,"+
	    //"PDB_NAME_*,"+
	"PDB_OUTPUT_DIRECTORY,"+
	    //"PDB_OUTPUT_DIRECTORY_*,"+
	"POSITION_INDEPENDENT_CODE,"+
	"POST_INSTALL_SCRIPT,"+
	"PREFIX,"+
	"PRE_INSTALL_SCRIPT,"+
	"PRIVATE_HEADER,"+
	"PROJECT_LABEL,"+
	"PUBLIC_HEADER,"+
	"RESOURCE,"+
	"RULE_LAUNCH_COMPILE,"+
	"RULE_LAUNCH_CUSTOM,"+
	"RULE_LAUNCH_LINK,"+
	"RUNTIME_OUTPUT_DIRECTORY,"+
	    //"RUNTIME_OUTPUT_DIRECTORY_*,"+
	"RUNTIME_OUTPUT_NAME,"+
	    //"RUNTIME_OUTPUT_NAME_*,"+
	"SKIP_BUILD_RPATH,"+
	"SOURCES,"+
	"SOVERSION,"+
	"STATIC_LIBRARY_FLAGS,"+
	    //"STATIC_LIBRARY_FLAGS_*,"+
	"SUFFIX,"+
	"TYPE,"+
	"VERSION,"+
	"VISIBILITY_INLINES_HIDDEN,"+
	"VS_DOTNET_REFERENCES,"+
	"VS_DOTNET_TARGET_FRAMEWORK_VERSION,"+
	    //"VS_GLOBAL_*,"+
	"VS_GLOBAL_KEYWORD,"+
	"VS_GLOBAL_PROJECT_TYPES,"+
	"VS_GLOBAL_ROOTNAMESPACE,"+
	"VS_KEYWORD,"+
	"VS_SCC_AUXPATH,"+
	"VS_SCC_LOCALPATH,"+
	"VS_SCC_PROJECTNAME,"+
	"VS_SCC_PROVIDER,"+
	"VS_WINRT_EXTENSIONS,"+
	"VS_WINRT_REFERENCES,"+
	"WIN32_EXECUTABLE,"+
	    //"XCODE_ATTRIBUTE_*,"+
	"ATTACHED_FILES,"+
	"ATTACHED_FILES_ON_FAIL,"+
	"COST,"+
	"DEPENDS,"+
	"ENVIRONMENT,"+
	"FAIL_REGULAR_EXPRESSION,"+
	"LABELS,"+
	"MEASUREMENT,"+
	"PASS_REGULAR_EXPRESSION,"+
	"PROCESSORS,"+
	"REQUIRED_FILES,"+
	"RESOURCE_LOCK,"+
	"RUN_SERIAL,"+
	"TIMEOUT,"+
	"WILL_FAIL,"+
	"WORKING_DIRECTORY,"+
	"ABSTRACT,"+
	"COMPILE_DEFINITIONS,"+
	    //"COMPILE_DEFINITIONS_*,"+
	"COMPILE_FLAGS,"+
	"EXTERNAL_OBJECT,"+
	"Fortran_FORMAT,"+
	"GENERATED,"+
	"HEADER_FILE_ONLY,"+
	"KEEP_EXTENSION,"+
	"LABELS,"+
	"LANGUAGE,"+
	"LOCATION,"+
	"MACOSX_PACKAGE_LOCATION,"+
	"OBJECT_DEPENDS,"+
	"OBJECT_OUTPUTS,"+
	"SYMBOLIC,"+
	"WRAP_EXCLUDE,"+
	"ADVANCED,"+
	"HELPSTRING,"+
	"MODIFIED,"+
	"STRINGS,"+
	"TYPE,"+
	"VALUE"
	);

    // cmake --help-module-list
    define_keywords(SYNK_FUNCTION,
	"AddFileDependencies,"+
	"BundleUtilities,"+
	"CMakeAddFortranSubdirectory,"+
	"CMakeBackwardCompatibilityCXX,"+
	"CMakeDependentOption,"+
	"CMakeDetermineVSServicePack,"+
	"CMakeExpandImportedTargets,"+
	"CMakeFindFrameworks,"+
	"CMakeFindPackageMode,"+
	"CMakeForceCompiler,"+
	"CMakeGraphVizOptions,"+
	"CMakePackageConfigHelpers,"+
	"CMakeParseArguments,"+
	"CMakePrintHelpers,"+
	"CMakePrintSystemInformation,"+
	"CMakePushCheckState,"+
	"CMakeVerifyManifest,"+
	"CPack,"+
	"CPackBundle,"+
	"CPackComponent,"+
	"CPackCygwin,"+
	"CPackDMG,"+
	"CPackDeb,"+
	"CPackNSIS,"+
	"CPackPackageMaker,"+
	"CPackRPM,"+
	"CPackWIX,"+
	"CTest,"+
	"CTestScriptMode,"+
	"CTestUseLaunchers,"+
	"CheckCCompilerFlag,"+
	"CheckCSourceCompiles,"+
	"CheckCSourceRuns,"+
	"CheckCXXCompilerFlag,"+
	"CheckCXXSourceCompiles,"+
	"CheckCXXSourceRuns,"+
	"CheckCXXSymbolExists,"+
	"CheckFortranFunctionExists,"+
	"CheckFunctionExists,"+
	"CheckIncludeFile,"+
	"CheckIncludeFileCXX,"+
	"CheckIncludeFiles,"+
	"CheckLanguage,"+
	"CheckLibraryExists,"+
	"CheckPrototypeDefinition,"+
	"CheckStructHasMember,"+
	"CheckSymbolExists,"+
	"CheckTypeSize,"+
	"CheckVariableExists,"+
	"Dart,"+
	"DeployQt4,"+
	"Documentation,"+
	"ExternalData,"+
	"ExternalProject,"+
	"FLTKConfig,"+
	"FeatureSummary,"+
	"FindALSA,"+
	"FindASPELL,"+
	"FindAVIFile,"+
	"FindArmadillo,"+
	"FindBISON,"+
	"FindBLAS,"+
	"FindBZip2,"+
	"FindBoost,"+
	"FindBullet,"+
	"FindCABLE,"+
	"FindCUDA,"+
	"FindCURL,"+
	"FindCVS,"+
	"FindCoin3D,"+
	"FindCups,"+
	"FindCurses,"+
	"FindCxxTest,"+
	"FindCygwin,"+
	"FindDCMTK,"+
	"FindDart,"+
	"FindDevIL,"+
	"FindDoxygen,"+
	"FindEXPAT,"+
	"FindFLEX,"+
	"FindFLTK,"+
	"FindFLTK2,"+
	"FindFreetype,"+
	"FindGCCXML,"+
	"FindGDAL,"+
	"FindGIF,"+
	"FindGLEW,"+
	"FindGLUT,"+
	"FindGTK,"+
	"FindGTK2,"+
	"FindGTest,"+
	"FindGettext,"+
	"FindGit,"+
	"FindGnuTLS,"+
	"FindGnuplot,"+
	"FindHDF5,"+
	"FindHSPELL,"+
	"FindHTMLHelp,"+
	"FindHg,"+
	"FindITK,"+
	"FindIcotool,"+
	"FindImageMagick,"+
	"FindJNI,"+
	"FindJPEG,"+
	"FindJasper,"+
	"FindJava,"+
	"FindKDE3,"+
	"FindKDE4,"+
	"FindLAPACK,"+
	"FindLATEX,"+
	"FindLibArchive,"+
	"FindLibLZMA,"+
	"FindLibXml2,"+
	"FindLibXslt,"+
	"FindLua50,"+
	"FindLua51,"+
	"FindMFC,"+
	"FindMPEG,"+
	"FindMPEG2,"+
	"FindMPI,"+
	"FindMatlab,"+
	"FindMotif,"+
	"FindOpenAL,"+
	"FindOpenGL,"+
	"FindOpenMP,"+
	"FindOpenSSL,"+
	"FindOpenSceneGraph,"+
	"FindOpenThreads,"+
	"FindPHP4,"+
	"FindPNG,"+
	"FindPackageHandleStandardArgs,"+
	"FindPackageMessage,"+
	"FindPerl,"+
	"FindPerlLibs,"+
	"FindPhysFS,"+
	"FindPike,"+
	"FindPkgConfig,"+
	"FindPostgreSQL,"+
	"FindProducer,"+
	"FindProtobuf,"+
	"FindPythonInterp,"+
	"FindPythonLibs,"+
	"FindQt,"+
	"FindQt3,"+
	"FindQt4,"+
	"FindQuickTime,"+
	"FindRTI,"+
	"FindRuby,"+
	"FindSDL,"+
	"FindSDL_image,"+
	"FindSDL_mixer,"+
	"FindSDL_net,"+
	"FindSDL_sound,"+
	"FindSDL_ttf,"+
	"FindSWIG,"+
	"FindSelfPackers,"+
	"FindSquish,"+
	"FindSubversion,"+
	"FindTCL,"+
	"FindTIFF,"+
	"FindTclStub,"+
	"FindTclsh,"+
	"FindThreads,"+
	"FindUnixCommands,"+
	"FindVTK,"+
	"FindWget,"+
	"FindWish,"+
	"FindX11,"+
	"FindXMLRPC,"+
	"FindZLIB,"+
	"Findlibproxy,"+
	"Findosg,"+
	"FindosgAnimation,"+
	"FindosgDB,"+
	"FindosgFX,"+
	"FindosgGA,"+
	"FindosgIntrospection,"+
	"FindosgManipulator,"+
	"FindosgParticle,"+
	"FindosgPresentation,"+
	"FindosgProducer,"+
	"FindosgQt,"+
	"FindosgShadow,"+
	"FindosgSim,"+
	"FindosgTerrain,"+
	"FindosgText,"+
	"FindosgUtil,"+
	"FindosgViewer,"+
	"FindosgVolume,"+
	"FindosgWidget,"+
	"Findosg_functions,"+
	"FindwxWidgets,"+
	"FindwxWindows,"+
	"FortranCInterface,"+
	"GNUInstallDirs,"+
	"GenerateExportHeader,"+
	"GetPrerequisites,"+
	"InstallRequiredSystemLibraries,"+
	"MacroAddFileDependencies,"+
	"ProcessorCount,"+
	"Qt4ConfigDependentSettings,"+
	"Qt4Macros,"+
	"SelectLibraryConfigurations,"+
	"SquishTestScript,"+
	"TestBigEndian,"+
	"TestCXXAcceptsFlag,"+
	"TestForANSIForScope,"+
	"TestForANSIStreamHeaders,"+
	"TestForSSTREAM,"+
	"TestForSTDNamespace,"+
	"UseEcos,"+
	"UseJava,"+
	"UseJavaClassFilelist,"+
	"UseJavaSymlinks,"+
	"UsePkgConfig,"+
	"UseQt4,"+
	"UseSWIG,"+
	"Use_wxWindows,"+
	"UsewxWidgets,"+
	"WriteBasicConfigVersionFile"
        );

    // cmake --help-variable-list
    define_keywords(SYNK_DEFINITION,
	"CMAKE_AR,"+
	"CMAKE_ARGC,"+
	"CMAKE_ARGV0,"+
	"CMAKE_BINARY_DIR,"+
	"CMAKE_BUILD_TOOL,"+
	"CMAKE_CACHEFILE_DIR,"+
	"CMAKE_CACHE_MAJOR_VERSION,"+
	"CMAKE_CACHE_MINOR_VERSION,"+
	"CMAKE_CACHE_PATCH_VERSION,"+
	"CMAKE_CFG_INTDIR,"+
	"CMAKE_COMMAND,"+
	"CMAKE_CROSSCOMPILING,"+
	"CMAKE_CTEST_COMMAND,"+
	"CMAKE_CURRENT_BINARY_DIR,"+
	"CMAKE_CURRENT_LIST_DIR,"+
	"CMAKE_CURRENT_LIST_FILE,"+
	"CMAKE_CURRENT_LIST_LINE,"+
	"CMAKE_CURRENT_SOURCE_DIR,"+
	"CMAKE_DL_LIBS,"+
	"CMAKE_EDIT_COMMAND,"+
	"CMAKE_EXECUTABLE_SUFFIX,"+
	"CMAKE_EXTRA_GENERATOR,"+
	"CMAKE_EXTRA_SHARED_LIBRARY_SUFFIXES,"+
	"CMAKE_GENERATOR,"+
	"CMAKE_GENERATOR_TOOLSET,"+
	"CMAKE_HOME_DIRECTORY,"+
	"CMAKE_IMPORT_LIBRARY_PREFIX,"+
	"CMAKE_IMPORT_LIBRARY_SUFFIX,"+
	"CMAKE_LINK_LIBRARY_SUFFIX,"+
	"CMAKE_MAJOR_VERSION,"+
	"CMAKE_MAKE_PROGRAM,"+
	"CMAKE_MINIMUM_REQUIRED_VERSION,"+
	"CMAKE_MINOR_VERSION,"+
	"CMAKE_PARENT_LIST_FILE,"+
	"CMAKE_PATCH_VERSION,"+
	"CMAKE_PROJECT_NAME,"+
	"CMAKE_RANLIB,"+
	"CMAKE_ROOT,"+
	"CMAKE_SCRIPT_MODE_FILE,"+
	"CMAKE_SHARED_LIBRARY_PREFIX,"+
	"CMAKE_SHARED_LIBRARY_SUFFIX,"+
	"CMAKE_SHARED_MODULE_PREFIX,"+
	"CMAKE_SHARED_MODULE_SUFFIX,"+
	"CMAKE_SIZEOF_VOID_P,"+
	"CMAKE_SKIP_RPATH,"+
	"CMAKE_SOURCE_DIR,"+
	"CMAKE_STANDARD_LIBRARIES,"+
	"CMAKE_STATIC_LIBRARY_PREFIX,"+
	"CMAKE_STATIC_LIBRARY_SUFFIX,"+
	"CMAKE_TWEAK_VERSION,"+
	"CMAKE_VERBOSE_MAKEFILE,"+
	"CMAKE_VERSION,"+
	"CMAKE_VS_PLATFORM_TOOLSET,"+
	"CMAKE_XCODE_PLATFORM_TOOLSET,"+
	"PROJECT_BINARY_DIR,"+
	"PROJECT_NAME,"+
	"PROJECT_SOURCE_DIR,"+
	    //"*_BINARY_DIR,"+
	    //"*_SOURCE_DIR,"+
	"BUILD_SHARED_LIBS,"+
	"CMAKE_ABSOLUTE_DESTINATION_FILES,"+
	"CMAKE_AUTOMOC_RELAXED_MODE,"+
	"CMAKE_BACKWARDS_COMPATIBILITY,"+
	"CMAKE_BUILD_TYPE,"+
	"CMAKE_COLOR_MAKEFILE,"+
	"CMAKE_CONFIGURATION_TYPES,"+
	"CMAKE_DEBUG_TARGET_PROPERTIES,"+
	    //"CMAKE_DISABLE_FIND_PACKAGE_*,"+
	"CMAKE_ERROR_DEPRECATED,"+
	"CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION,"+
	"CMAKE_FIND_LIBRARY_PREFIXES,"+
	"CMAKE_FIND_LIBRARY_SUFFIXES,"+
	"CMAKE_FIND_PACKAGE_WARN_NO_MODULE,"+
	"CMAKE_IGNORE_PATH,"+
	"CMAKE_INCLUDE_PATH,"+
	"CMAKE_INSTALL_DEFAULT_COMPONENT_NAME,"+
	"CMAKE_INSTALL_PREFIX,"+
	"CMAKE_LIBRARY_PATH,"+
	"CMAKE_MFC_FLAG,"+
	"CMAKE_MODULE_PATH,"+
	"CMAKE_NOT_USING_CONFIG_FLAGS,"+
	"CMAKE_POLICY_DEFAULT_CMP+,"+
	"CMAKE_PREFIX_PATH,"+
	"CMAKE_PROGRAM_PATH,"+
	"CMAKE_SKIP_INSTALL_ALL_DEPENDENCY,"+
	"CMAKE_SYSTEM_IGNORE_PATH,"+
	"CMAKE_SYSTEM_INCLUDE_PATH,"+
	"CMAKE_SYSTEM_LIBRARY_PATH,"+
	"CMAKE_SYSTEM_PREFIX_PATH,"+
	"CMAKE_SYSTEM_PROGRAM_PATH,"+
	"CMAKE_USER_MAKE_RULES_OVERRIDE,"+
	"CMAKE_WARN_DEPRECATED,"+
	"CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION,"+
	"APPLE,"+
	"BORLAND,"+
	"CMAKE_CL_64,"+
	"CMAKE_COMPILER_2005,"+
	"CMAKE_HOST_APPLE,"+
	"CMAKE_HOST_SYSTEM,"+
	"CMAKE_HOST_SYSTEM_NAME,"+
	"CMAKE_HOST_SYSTEM_PROCESSOR,"+
	"CMAKE_HOST_SYSTEM_VERSION,"+
	"CMAKE_HOST_UNIX,"+
	"CMAKE_HOST_WIN32,"+
	"CMAKE_LIBRARY_ARCHITECTURE,"+
	"CMAKE_LIBRARY_ARCHITECTURE_REGEX,"+
	"CMAKE_OBJECT_PATH_MAX,"+
	"CMAKE_SYSTEM,"+
	"CMAKE_SYSTEM_NAME,"+
	"CMAKE_SYSTEM_PROCESSOR,"+
	"CMAKE_SYSTEM_VERSION,"+
	"CYGWIN,"+
	"ENV,"+
	"MSVC,"+
	"MSVC10,"+
	"MSVC11,"+
	"MSVC12,"+
	"MSVC60,"+
	"MSVC70,"+
	"MSVC71,"+
	"MSVC80,"+
	"MSVC90,"+
	"MSVC_IDE,"+
	"MSVC_VERSION,"+
	"UNIX,"+
	"WIN32,"+
	"XCODE_VERSION,"+
	    //"CMAKE_*_POSTFIX,"+
	    //"CMAKE_*_VISIBILITY_PRESET,"+
	"CMAKE_ARCHIVE_OUTPUT_DIRECTORY,"+
	"CMAKE_AUTOMOC,"+
	"CMAKE_AUTOMOC_MOC_OPTIONS,"+
	"CMAKE_BUILD_WITH_INSTALL_RPATH,"+
	"CMAKE_DEBUG_POSTFIX,"+
	"CMAKE_EXE_LINKER_FLAGS,"+
	    //"CMAKE_EXE_LINKER_FLAGS_*,"+
	"CMAKE_Fortran_FORMAT,"+
	"CMAKE_Fortran_MODULE_DIRECTORY,"+
	"CMAKE_GNUtoMS,"+
	"CMAKE_INCLUDE_CURRENT_DIR,"+
	"CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE,"+
	"CMAKE_INSTALL_NAME_DIR,"+
	"CMAKE_INSTALL_RPATH,"+
	"CMAKE_INSTALL_RPATH_USE_LINK_PATH,"+
	"CMAKE_LIBRARY_OUTPUT_DIRECTORY,"+
	"CMAKE_LIBRARY_PATH_FLAG,"+
	    //"CMAKE_LINK_DEF_FILE_FLAG,"+
	"CMAKE_LINK_DEPENDS_NO_SHARED,"+
	"CMAKE_LINK_INTERFACE_LIBRARIES,"+
	"CMAKE_LINK_LIBRARY_FILE_FLAG,"+
	"CMAKE_LINK_LIBRARY_FLAG,"+
	"CMAKE_MACOSX_BUNDLE,"+
	"CMAKE_MODULE_LINKER_FLAGS,"+
	    //"CMAKE_MODULE_LINKER_FLAGS_*,"+
	"CMAKE_NO_BUILTIN_CHRPATH,"+
	"CMAKE_PDB_OUTPUT_DIRECTORY,"+
	"CMAKE_POSITION_INDEPENDENT_CODE,"+
	"CMAKE_RUNTIME_OUTPUT_DIRECTORY,"+
	"CMAKE_SHARED_LINKER_FLAGS,"+
	    //"CMAKE_SHARED_LINKER_FLAGS_*,"+
	"CMAKE_SKIP_BUILD_RPATH,"+
	"CMAKE_SKIP_INSTALL_RPATH,"+
	"CMAKE_STATIC_LINKER_FLAGS,"+
	    //"CMAKE_STATIC_LINKER_FLAGS_*,"+
	"CMAKE_TRY_COMPILE_CONFIGURATION,"+
	"CMAKE_USE_RELATIVE_PATHS,"+
	"CMAKE_VISIBILITY_INLINES_HIDDEN,"+
	"CMAKE_WIN32_EXECUTABLE,"+
	"EXECUTABLE_OUTPUT_PATH,"+
	"LIBRARY_OUTPUT_PATH,"+
	    //"CMAKE_*_ARCHIVE_APPEND,"+
	    //"CMAKE_*_ARCHIVE_CREATE,"+
	    //"CMAKE_*_ARCHIVE_FINISH,"+
	    //"CMAKE_*_COMPILER,"+
	    //"CMAKE_*_COMPILER_ABI,"+
	    //"CMAKE_*_COMPILER_ID,"+
	    //"CMAKE_*_COMPILER_LOADED,"+
	    //"CMAKE_*_COMPILER_VERSION,"+
	    //"CMAKE_*_COMPILE_OBJECT,"+
	    //"CMAKE_*_CREATE_SHARED_LIBRARY,"+
	    //"CMAKE_*_CREATE_SHARED_MODULE,"+
	    //"CMAKE_*_CREATE_STATIC_LIBRARY,"+
	    //"CMAKE_*_FLAGS,"+
	    //"CMAKE_*_FLAGS_DEBUG,"+
	    //"CMAKE_*_FLAGS_MINSIZEREL,"+
	    //"CMAKE_*_FLAGS_RELEASE,"+
	    //"CMAKE_*_FLAGS_RELWITHDEBINFO,"+
	    //"CMAKE_*_IGNORE_EXTENSIONS,"+
	    //"CMAKE_*_IMPLICIT_INCLUDE_DIRECTORIES,"+
	    //"CMAKE_*_IMPLICIT_LINK_DIRECTORIES,"+
	    //"CMAKE_*_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES,"+
	    //"CMAKE_*_IMPLICIT_LINK_LIBRARIES,"+
	    //"CMAKE_*_LIBRARY_ARCHITECTURE,"+
	    //"CMAKE_*_LINKER_PREFERENCE,"+
	    //"CMAKE_*_LINKER_PREFERENCE_PROPAGATES,"+
	    //"CMAKE_*_LINK_EXECUTABLE,"+
	    //"CMAKE_*_OUTPUT_EXTENSION,"+
	    //"CMAKE_*_PLATFORM_ID,"+
	    //"CMAKE_*_SIZEOF_DATA_PTR,"+
	    //"CMAKE_*_SOURCE_FILE_EXTENSIONS,"+
	"CMAKE_COMPILER_IS_GNU+,"+
	"CMAKE_Fortran_MODDIR_DEFAULT,"+
	"CMAKE_Fortran_MODDIR_FLAG,"+
	"CMAKE_Fortran_MODOUT_FLAG,"+
	"CMAKE_INTERNAL_PLATFORM_ABI"
	    //"CMAKE_USER_MAKE_RULES_OVERRIDE_*"
        );

    set_syntax_flags(SYNF_HILITE_LINECONT);
}


/*
 *  Modeline support
 */
string
_cmake_mode()
{
    return MODENAME;
}


/*
 *  Package support
 */
string
_cmake_highlight_first()
{
    attach_syntax(MODENAME);
    return "";
}

/*end*/