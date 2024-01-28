import { glob } from 'glob'
import fs from 'fs';
import path from 'path';

const BUILD_PATH = "build";
const COMPILER_PATH = "clang";
const OUT_FILE = "compile_commands.json";
const workspaceFolder = path.join('C:', 'Users', 'mikes', 'Desktop', 'Desktop', 'Making_Games', 'Game5', 'Code');
const COMPILER_ARGS = [
  '-std=c++17',
  `-I${path.join('C:', 'VulkanSDK', '1.3.236.0', 'Include')}`,
  `-I${path.join(workspaceFolder, 'vendor', 'sdl-2.26.1', 'include')}`,
  `-I${path.join(workspaceFolder, 'vendor', 'sdl-mixer-2.6.2', 'include')}`,
  `-I${path.join(workspaceFolder, 'vendor', 'glm-0.9.9.8')}`,
  `-I${path.join(workspaceFolder, 'vendor', 'tinyobjloader', 'include')}`,
  `-I${path.join(workspaceFolder, 'vendor', 'stb')}`,
  `-I${path.join(workspaceFolder, 'vendor', 'protobuf-25.2', 'include')}`,
  `-I${path.join(workspaceFolder, 'vendor', 'lua-5.4.2', 'include')}`,
];

console.log("scanning directory...");
const unit_files = await glob('{src,tests}/**/*.cpp');

console.debug("unit_files: ", unit_files);

const compile_commands = [];

for (const unit_file of unit_files) {
  compile_commands.push({
    directory: BUILD_PATH,
    arguments: [
      COMPILER_PATH,
      ...COMPILER_ARGS,
      "-c",
      "-o", `${path.basename(unit_file, '.cpp')}.o`,
      path.join('..', unit_file),
    ],
    file: path.join(workspaceFolder, unit_file),
  });
}

console.log(`writing ${OUT_FILE}...`)
fs.writeFileSync(OUT_FILE, JSON.stringify(compile_commands, null, 2));

console.log("done.");