import os, re, sys
from conans import tools

class conanPackages:
    def __createFolderDownload (self, v_downloadsPath):
        print ('createFolderDownload')
        if not os.path.isdir (v_downloadsPath):
            os.mkdir (v_downloadsPath)
        os.chdir (v_downloadsPath)

    def __cloneRepo (self, v_name, v_version, v_downloadsPath, v_repoUrl):
        print ('cloneRepo')
        url = v_repoUrl + '/' + v_name + '.git'
        print ('url', url)

        packageDownloadsPath = v_downloadsPath + '/' + v_name
        if not os.path.isdir (packageDownloadsPath):
            self.run ('git clone --branch ' + v_version + ' ' + url)
        os.chdir (packageDownloadsPath + '/Conan')

    def __createPackage (self, v_user, v_channel):
        print ('createPackage')
        self.run('conan create . ' + v_user + '/' + v_channel)
        
    def __parse (self, v_package):
        packageComponent = (re.split('[/@]', v_package, 3))
        return {'name' : packageComponent [0], 'version' : packageComponent [1], 'user' : packageComponent [2], 'channel' : packageComponent [3]}

    def getPaths (self, v_packagesPath, v_packages):
        print ('getPaths')
        paths        = {}
        packageNames = []
        for package in v_packages:
            print ("parse: ", package)
            packageComponent   = conanPackages.__parse (self, package)
            path               = v_packagesPath + '/' + packageComponent ['name'] + '/' + packageComponent ['version'] + '/' + packageComponent ['user'] + '/' + packageComponent ['channel'] + '/package'
            hashFolder         = os.listdir (path)
            packageIncludePath = path + '/' + hashFolder [0] + '/include'
            packageLibPath     = path + '/' + hashFolder [0] + '/lib'
            packageName        = 'lib' + packageComponent ['name'] + 'Lib.a'
             
            if not os.path.isdir (packageIncludePath):
                raise Exception ('%s. Is not package include path', packageIncludePath)

            if not os.path.isdir (packageLibPath):
                raise Exception ('%s. Is not package include lib path', packageLibPath)

            paths [packageComponent ['name'] + 'PackageIncludePath'] = packageIncludePath
            paths [packageComponent ['name'] + 'PackageLibPath']     = packageLibPath
            paths [packageComponent ['name'] + 'PackageName']        = packageName
            packageNames.append (packageComponent ['name'])

        tools.replace_in_file (os.getcwd ().replace ('/Conan','') + "/CMakeLists.txt", "PackagesTempNames", str (packageNames).strip ('[]').replace (',','').replace ('\'', ''), False)
        return paths

    def install (self, v_downloadsPath, v_repoUrl, v_packages):
        print ('install')
        for package in v_packages:
            packageComponent = conanPackages.__parse (self, package)
            conanPackages.__createFolderDownload     (self, v_downloadsPath)
            conanPackages.__cloneRepo                (self, packageComponent ['name'], packageComponent ['version'], v_downloadsPath, v_repoUrl)
            conanPackages.__createPackage            (self, packageComponent ['user'], packageComponent ['channel'])
