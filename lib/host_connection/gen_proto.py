import os
import shutil
import sys

NANOPB_GEN_BIN = 'nanopb_generator.py'
PROTO_DEF_DIR = '/proto/proto_definition'
PROTO_OUR_C_DIR = '/proto/out_c'
DEFAULT_PROJECT = "backpack"


if sys.platform == 'win32':
  NANOPB_GEN_DIR = os.path.dirname('third_parties/protobuf/nanopb/nanopb-0.4.7-windows-x86/generator/')
  MONITOR_APP_PROTO_DIR = os.path.join('C:/working/project/boston/source/backpack_app/proto_definition')
  # Use script file
  NANOPB_GEN_CMD = 'python3 ' + os.path.join(NANOPB_GEN_DIR, NANOPB_GEN_BIN)
  # Use binary file
  # NANOPB_GEN_CMD = './third_parties/protobuf/nanopb/nanopb-0.4.7-windows-x86/generator-bin/nanopb_generator'
elif sys.platform == "darwin":
  NANOPB_GEN_DIR = os.path.dirname('third_parties/protobuf/nanopb/nanopb-0.4.7-macosx-x86/generator/')
  MONITOR_APP_PROTO_DIR = os.path.join('/Users/toanhuynh/working/project/backpack/backpack_app/proto_definition')
  NANOPB_GEN_CMD = './third_parties/protobuf/nanopb/nanopb-0.4.7-macosx-x86/generator-bin/nanopb_generator'
else:
  print("Don't support on this platform")
  exit(1)

def get_proto_file(dir):
  # Get all files .proto in dir then return list
  proto_files = []
  for root, dirs, files in os.walk(dir):
    for file in files:
      if file.endswith('.proto'):
        print(file)
        proto_files.append(file)

  return proto_files


def main(project, app_dir):
  proto_dir = os.path.join("./projects/" + project + PROTO_DEF_DIR)
  out_c = os.path.join("./projects/" + project + PROTO_OUR_C_DIR)

  proto_files = get_proto_file(proto_dir)

  cmd = NANOPB_GEN_CMD + ' -I ' + proto_dir +' -D ' + out_c

  for proto_file in proto_files:
    cmd += ' ' + proto_file

  print(cmd)
  os.system(cmd)

  if os.path.exists(app_dir):
    # Copy *.proto to application
    shutil.copytree(proto_dir, MONITOR_APP_PROTO_DIR, dirs_exist_ok=True)
    # Copyt nanopb.proto to application
    shutil.copy(os.path.join(NANOPB_GEN_DIR,'proto','nanopb.proto'), MONITOR_APP_PROTO_DIR)
    # Copy folder generator\proto\google\protobuf to application
    shutil.copytree(os.path.join(NANOPB_GEN_DIR,'proto','google','protobuf'), os.path.join(MONITOR_APP_PROTO_DIR,'google','protobuf'), dirs_exist_ok=True)

    print("Copied *.proto files to application!")
    print("Successfully!")
  else:
    print("No app folder found")


if __name__ == '__main__':
    # Get first argument
    project = DEFAULT_PROJECT
    app_dir = MONITOR_APP_PROTO_DIR

    if(len(sys.argv) < 2):
      print('Usage: python gen_proto.py <app_dir>')
    else:
      app_dir = sys.argv[1]

    print("Build proto project: " + project)
    main(project, app_dir)
