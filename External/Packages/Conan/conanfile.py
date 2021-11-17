from conans import ConanFile, CMake, tools
from conanPackages import conanPackages  
import os

class Conan(ConanFile):
    name            = "BleClient"
    version         = "1.0"
    user            = "ssitkowx"
    channel         = "stable"
    license         = "freeware"
    repoUrl         = "https://github.com/ssitkowx"
    url             = repoUrl + '/' + name + '.git'
    description     = "Template for projects and packages"
    settings        = "os", "compiler", "build_type", "arch"
    options         = {"shared": [True, False]}
    default_options = "shared=False"
    generators      = "cmake"
    author          = "sylsit"
    exports         = "*"
    exports_sources = '../*'
    requires        = ["gtest/1.8.1@bincrafters/stable"]
    packagesPath    = "/home/sylwester/.conan/data"
    downloadsPath   = "/home/sylwester/.conan/download"
    packages        = ["Gpio/1.0@ssitkowx/stable",
                       "Rtos/1.0@ssitkowx/stable",
                       "Utils/1.0@ssitkowx/stable",
                       "Logger/1.0@ssitkowx/stable",
                       "LoggerHw/1.0@ssitkowx/stable",
                       "SystemTime/1.0@ssitkowx/stable",
                       "Communication/1.0@ssitkowx/stable"]

    def source (self):   
        conanPackages.install (self, self.downloadsPath, self.repoUrl, self.packages)

    def build (self):
        projectPath  = os.getcwd ().replace ('/Conan','')
        buildPath = projectPath + '/Build'
        
        if not os.path.exists (projectPath + '/CMakeLists.txt'):
            projectPath = self.downloadsPath + '/' + self.name
            buildPath   = os.getcwd() + '/Build'
            
        tools.replace_in_file (projectPath + "/CMakeLists.txt", "PackageTempName", self.name, False)

        if self.settings.os == 'Linux' and self.settings.compiler == 'gcc':
            packagesPaths = conanPackages.getPaths (self, self.packagesPath, self.packages)
            cmake         = CMake(self)
            
            conanPath = os.getcwd () + "/packagesProperties.txt"
            packagesPropertiesFileHandler = open (conanPath, "w")
            for packagePathKey, packagePathValue in packagesPaths.items ():
                packagesPropertiesFileHandler.writelines (packagePathKey + "=" + packagePathValue + "\n")
            packagesPropertiesFileHandler.close ()
            
            cmake.configure (source_dir = projectPath, build_dir = buildPath)
            cmake.build ()
        else:
            raise Exception ('Unsupported platform or compiler')
        
    def package (self):   
        projectPath = os.getcwd ().replace ('/Conan','')
        
        if not os.path.exists (projectPath + '/CMakeLists.txt'):
            projectPath = self.downloadsPath + '/' + self.name
    
        self.copy ('*.h'     , dst = 'include', src = projectPath + '/Project' , keep_path = False)
        self.copy ('*.hxx'   , dst = 'include', src = projectPath + '/Project' , keep_path = False)
        self.copy ('*.lib'   , dst = 'lib'    , src = projectPath + '/Build/lib', keep_path = False)
        self.copy ('*.dll'   , dst = 'bin'    , src = projectPath + '/Build/bin', keep_path = False)
        self.copy ('*.dylib*', dst = 'lib'    , src = projectPath + '/Build/lib', keep_path = False)
        self.copy ('*.so'    , dst = 'lib'    , src = projectPath + '/Build/lib', keep_path = False)
        self.copy ('*.a'     , dst = 'lib'    , src = projectPath + '/Build/lib', keep_path = False)

    def package_info (self):
        self.cpp_info.libs = [self.name]
