from conans import ConanFile, tools
from pathlib import Path

class GSOCPackage(ConanFile):
    name = "provencore_gw"
    version = "5.1.0.0"
    user = "sdv_valeo_sweet500"
    channel = "release"
    description = """ Provencore package for s32g3xxaevb3 from tag PC_5.1.0.0 """
    url = "None"
    license = "None"
    author = "None"
    topics = None
    short_paths = False

    options = {
        "board": ["a-sample", "b0-sample", "b1-sample"],
    }

    default_options = {
        "board": "b1-sample",
    } 

    def package(self):
        self.copy("*",dst="files/", src="../../../gsoc/patches/atf/", keep_path=False, symlinks=True, excludes=None, ignore_case=False)
        self.copy("provencore.bin", src="../../../gsoc/provencore/build", keep_path=False, symlinks=True, excludes=None, ignore_case=False)
        
    
    def package_info(self):
        self.env_info.Provencore_GW_5_1_0_0_SDV_VALEO_SWEET500_RELEASE = self.package_folder
        self.env_info.Provencore_GW = self.package_folder
        self.cpp_info.libs = tools.collect_libs(self)