import { glob } from 'glob'
import fs from 'fs/promises';
import path from 'path';
import { spawn } from 'child_process';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const CONCURRENCY = 48; // threadripper ftw!
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
  await fs.writeFile(OUT_FILE, JSON.stringify(compile_commands, null, 2));

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

const promiseBatch = async function* (concurrency, list, fn) {
  for (let p = [], i = 0, l = list.length; i < l || p.length > 0;) {
    if (i < l) {
      let _p;
      _p = fn(list[i]).then(r => [_p.__id, r]);
      _p.__id = i++;
      if (p.push(_p) < concurrency) {
        continue;
      }
    }
    const [id, res] = await Promise.race(p);
    p = p.filter(x => x.__id !== id);
    yield res;
  }
};

const compile_test = async (basename) => {
  console.log(`compiling ${basename}...`);
  const absbuild = (...args) => path.join(workspaceFolder, BUILD_PATH, ...args);

  // compile translation units in parallel (N-at-once)
  const unit_files = [`tests/lib/${basename}.cpp`];
  for (const u of COMPILER_TRANSLATION_UNITS) {
    unit_files.push(...await glob(path.relative(workspaceFolder, absbuild(u)).replace(/\\/g, '/')));
  }
  const dsts = [];
  const compileTranslationUnit = async (unit) => {
    const RX_EXT = /\.[\w\d]{1,3}$/i;
    const dir = path.relative(process.cwd(), absbuild(path.dirname(unit)));
    await fs.mkdir(dir, { recursive: true });

    const src = rel(workspaceFolder, unit);
    const dst = rel(workspaceFolder, BUILD_PATH, unit.replace(RX_EXT, '.o'));
    dsts.push(dst);

    let dstExists = false;
    try {
      await fs.access(path.join(BUILD_PATH, dst), fs.constants.F_OK);
      dstExists = true;
    }
    catch (e) {
    }
    if (dstExists) {
      const srcStat = await fs.stat(path.join(BUILD_PATH, src));
      const dstStat = await fs.stat(path.join(BUILD_PATH, dst));
      if (srcStat.mtime < dstStat.mtime) {
        return;
      }
    }

    const code = await child_spawn(COMPILER_PATH, [
      ...DEBUG_COMPILER_ARGS,
      ...COMPILER_ARGS,
      src,
      '-c',
      '-o', dst,
    ]);

    return dst;
  };
  const objs = [];
  for await (const obj of promiseBatch(CONCURRENCY, unit_files, compileTranslationUnit)) {
    if (obj) {
      objs.push(obj);
    }
  }

  // linker stage
  let code = 0;
  if (objs.length > 0) {
    code = await child_spawn(COMPILER_PATH, [
      ...DEBUG_COMPILER_ARGS,
      ...COMPILER_ARGS,
      ...LINKER_LIBS,
      ...LINKER_LIB_PATHS,
      ...dsts,
      '-o', `${basename}.exe`,
    ]);
  }
  if (0 == code) {
    const code2 = await child_spawn(`${basename}.exe`);
  }
  console.log("done.");
};

const [, , cmd] = process.argv;
switch (cmd) {
  case 'compile_commands':
    await generate_clangd_compile_commands();
    break;
  case 'Audio_test':
  case 'Gamepad_test':
  case 'Lua_test':
  case 'Pong_test':
  case 'Protobuf_test':
  case 'Window_test':
    await compile_test(cmd);
    break;
  case 'help':
  default:
    console.log(`
Mike's hand-rolled build system.

USAGE:
  node build_scripts\\Makefile.mjs <SUBCOMMAND>

SUBCOMMANDS:
  compile_commands
    Generate the .json file needed for clangd for vscode extension
  Audio_test
    Test SDL audio integration
  Gamepad_test
    Test SDL gamepad integration
  Lua_test
    Test Lua sandbox integration
  Pong_test
    Test everything (game demo)
  Protobuf_test
    Test Google Protobuf data read/write
  Window_test
    Test SDL window integration
`);
    break;
}
