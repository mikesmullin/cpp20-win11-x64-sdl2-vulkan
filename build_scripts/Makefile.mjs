import { glob } from 'glob'
import fs from 'fs';
import path from 'path';
import { spawn } from 'child_process';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const BUILD_PATH = "build";
const COMPILER_PATH = "clang++";
const OUT_FILE = "compile_commands.json";
const abs = (...args) => path.join(...args);
const workspaceFolder = path.join(__dirname, '..');
const rel = (...args) =>
  path.relative(path.join(workspaceFolder, BUILD_PATH), path.join(...args));
const DEBUG_COMPILER_ARGS = [
  '-O0',
  '-gdwarf', // adds gdb support
  // TODO: do we need to pass `-debug` to the linker? `-Xlinker -debug`?
];
const COMPILER_ARGS = [
  '-m64',
  '-std=c++23',
  `-I${abs('C:', 'VulkanSDK', '1.3.236.0', 'Include')}`,
  `-I${rel(workspaceFolder, 'vendor', 'sdl-2.26.1', 'include')}`,
  `-I${rel(workspaceFolder, 'vendor', 'sdl-mixer-2.6.2', 'include')}`,
  `-I${rel(workspaceFolder, 'vendor', 'glm-0.9.9.8')}`,
  `-I${rel(workspaceFolder, 'vendor', 'tinyobjloader', 'include')}`,
  `-I${rel(workspaceFolder, 'vendor', 'stb')}`,
  `-I${rel(workspaceFolder, 'vendor', 'protobuf-25.2', 'include')}`,
  `-I${rel(workspaceFolder, 'vendor', 'lua-5.4.2', 'include')}`,
];
const LINKER_LIBS = [
  '-luser32',
  '-lshell32',
  '-lgdi32',
];
const LINKER_LIB_PATHS = [
  `-L${rel(workspaceFolder, 'vendor', 'sdl-2.26.1', 'lib', 'x64')}`, '-lSDL2',
  `-L${rel(workspaceFolder, 'vendor', 'sdl-mixer-2.6.2', 'lib', 'x64')}`, '-lSDL2_mixer',
  `-L${abs('C:', 'VulkanSDK', '1.3.236.0', 'Lib')}`, '-lvulkan-1',
  `-L${rel(workspaceFolder, 'vendor', 'protobuf-25.2', 'x64')}`, '-llibprotobuf-lite',
  `-L${rel(workspaceFolder, 'vendor', 'lua-5.4.2', 'x64')}`, '-llua54',
];
const COMPILER_TRANSLATION_UNITS = [
  rel(workspaceFolder, 'src', 'components', '*.cpp'),
  rel(workspaceFolder, 'src', 'lib', '*.cpp'),
  rel(workspaceFolder, 'src', 'proto', '*.cc'),
];

const generate_clangd_compile_commands = async () => {
  console.log("scanning directory...");
  const unit_files = await glob('{src,tests}/**/*.cpp');

  console.debug("unit_files: ", unit_files);

  const compile_commands = [];

  for (const unit_file of unit_files) {
    compile_commands.push({
      directory: path.join(workspaceFolder, BUILD_PATH),
      arguments: [
        COMPILER_PATH,
        ...COMPILER_ARGS,
        "-c",
        "-o", `${path.basename(unit_file, '.cpp')}.o`,
        rel(unit_file),
      ],
      file: rel(workspaceFolder, unit_file),
    });
  }

  console.log(`writing ${OUT_FILE}...`)
  fs.writeFileSync(OUT_FILE, JSON.stringify(compile_commands, null, 2));

  console.log("done.");
};

const child_spawn = async (cmd, args = []) => {
  const cwd = path.relative(process.cwd(), path.join(workspaceFolder, BUILD_PATH));
  console.log(`cd ${cwd}`);
  console.log(`${cmd} ${args.join(' ')}`);
  const child = spawn(cmd, args, { cwd, stdio: 'inherit' });
  const code = await new Promise((ok) => {
    child.on('close', (code) => {
      if (code !== 0) {
        console.log(`process exited with code ${code}`);
      }
      ok(code);
    });
  });
  return code;
};

const compile_pong_test = async () => {
  console.log("compiling...");
  const code = await child_spawn(COMPILER_PATH, [
    ...DEBUG_COMPILER_ARGS,
    ...COMPILER_ARGS,
    ...LINKER_LIBS,
    ...LINKER_LIB_PATHS,
    rel(workspaceFolder, 'tests', 'lib', 'Pong_test.cpp'),
    ...COMPILER_TRANSLATION_UNITS,
    '-oPong_test.exe',
  ]);
  if (0 == code) {
    const code2 = await child_spawn('Pong_test.exe');
  }
  console.log("done.");
};

const [, , cmd] = process.argv;
switch (cmd) {
  case 'compile_commands':
    await generate_clangd_compile_commands();
    break;
  case 'Pong_test':
    await compile_pong_test();
    break;
}
