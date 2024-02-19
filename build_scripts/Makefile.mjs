import { glob } from 'glob'
import cbfs from 'fs';
import fs from 'fs/promises';
import path from 'path';
import { spawn } from 'child_process';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const CONCURRENCY = 48; // threadripper ftw!
const GLSLC_PATH = path.join('C:', 'VulkanSDK', '1.3.236.0', 'Bin', 'glslc.exe');
const BUILD_PATH = "build";
const COMPILER_PATH = "clang++";
const C_COMPILER_PATH = 'clang';
const RX_EXT = /\.[\w\d]{1,3}$/i;
const RX_C = /\.c$/i;
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
const CPP_COMPILER_ARGS = [
  '-std=c++23',
];
const COMPILER_ARGS = [
  '-m64',
  `-I${abs('C:', 'VulkanSDK', '1.3.236.0', 'Include')}`,
  `-I${rel(workspaceFolder, 'vendor', 'sdl-2.26.1', 'include')}`,
  `-I${rel(workspaceFolder, 'vendor', 'glm-0.9.9.8')}`,
  `-I${rel(workspaceFolder, 'vendor', 'tinyobjloader', 'include')}`,
  `-I${rel(workspaceFolder, 'vendor', 'stb')}`,
  `-I${rel(workspaceFolder, 'vendor', 'protobuf-25.2', 'include')}`,
  `-I${rel(workspaceFolder, 'vendor', 'lua-5.4.2', 'include')}`,
  `-I${rel(workspaceFolder, 'vendor', 'rxi', 'cmixer-3592e0e', 'include')}`,
];
const LINKER_LIBS = [
  '-luser32',
  '-lshell32',
  '-lgdi32',
];
const LINKER_LIB_PATHS = [
  `-L${rel(workspaceFolder, 'vendor', 'sdl-2.26.1', 'lib', 'x64')}`, '-lSDL2',
  `-L${abs('C:', 'VulkanSDK', '1.3.236.0', 'Lib')}`, '-lvulkan-1',
  `-L${rel(workspaceFolder, 'vendor', 'protobuf-25.2', 'x64')}`, '-llibprotobuf-lite',
  `-L${rel(workspaceFolder, 'vendor', 'lua-5.4.2', 'x64')}`, '-llua54',
];
const COMPILER_TRANSLATION_UNITS = [
  rel(workspaceFolder, 'src', 'components', '*.cpp'),
  rel(workspaceFolder, 'src', 'lib', '*.cpp'),
  rel(workspaceFolder, 'src', 'proto', '*.cc'),
  rel(workspaceFolder, 'vendor', 'rxi', 'cmixer-3592e0e', 'include', '*.c'),
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
        ...CPP_COMPILER_ARGS,
        ...COMPILER_ARGS,
        "-c",
        "-o", `${path.basename(unit_file).replace(RX_EXT, '.o')}`,
        rel(unit_file),
      ],
      file: rel(workspaceFolder, unit_file),
    });
  }

  console.log(`writing ${OUT_FILE}...`)
  await fs.writeFile(OUT_FILE, JSON.stringify(compile_commands, null, 2));

  console.log("done.");
};

const child_spawn = async (cmd, args = [], opts = {}) => {
  const cwd = path.relative(process.cwd(), path.join(workspaceFolder, BUILD_PATH));
  console.log(`cd ${cwd}`);
  console.log(`${opts.stdin ? `type ${opts.stdin} | ` : ''}${cmd} ${args.join(' ')}${opts.stdout ? ` > ${opts.stdout}` : ''}`);
  let infd, stdin, outfd, stdout;
  const stdio = ['inherit', 'inherit', 'inherit'];
  if (opts.stdin) {
    stdio[0] = 'pipe';
    stdin = cbfs.createReadStream(path.join(workspaceFolder, 'assets', 'proto', 'addressbook.pb'));
  }
  if (opts.stdout) {
    stdio[1] = 'pipe';
    stdout = await cbfs.createWriteStream(path.join(workspaceFolder, 'assets', 'proto', 'addressbook.bin'));
  }
  const child = spawn(cmd, args, { cwd, stdio });
  if (opts.stdin) {
    stdin.pipe(child.stdin);
  }
  if (opts.stdout) {
    child.stdout.pipe(stdout);
  }
  const code = await new Promise((ok) => {
    child.on('close', async (code) => {
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

const all = async () => {
  await clean();
  await copy_dlls();
  await shaders();
  await protobuf();
  await compile_test('Pong_test');
};

const copy_dlls = async () => {
  const srcs = [
    path.join(workspaceFolder, 'vendor', 'sdl-2.26.1', 'lib', 'x64', 'SDL2.dll'),
    path.join(workspaceFolder, 'vendor', 'protobuf-25.2', 'x64', 'libprotobuf-lite.dll'),
    path.join(workspaceFolder, 'vendor', 'lua-5.4.2', 'x64', 'lua54.dll'),
  ];
  const dest = path.join(workspaceFolder, BUILD_PATH);
  for (const src of srcs) {
    await fs.copyFile(src, path.join(dest, path.basename(src)));
  }
};

const shaders = async () => {
  await child_spawn(GLSLC_PATH,
    ['../assets/shaders/simple_shader.vert', '-o', '../assets/shaders/simple_shader.vert.spv']);

  await child_spawn(GLSLC_PATH,
    ['../assets/shaders/simple_shader.frag', '-o', '../assets/shaders/simple_shader.frag.spv']);
};

const protobuf = async () => {
  const PROTOC_PATH = path.join(workspaceFolder, 'vendor', 'protobuf-25.2', 'tools', 'protoc.exe');
  await child_spawn(PROTOC_PATH, [
    `--cpp_out=${path.join(workspaceFolder, 'src', 'proto')}`,
    '--proto_path', path.join(workspaceFolder, 'assets', 'proto'),
    'addressbook.proto',
  ]);

  await child_spawn(PROTOC_PATH, [
    '--encode', 'tutorial.AddressBook',
    '--proto_path', path.join(workspaceFolder, 'assets', 'proto'),
    'addressbook.proto',
  ], {
    stdin: path.join(workspaceFolder, 'assets', 'proto', 'addressbook.pb'),
    stdout: path.join(workspaceFolder, 'assets', 'proto', 'addressbook.bin'),
  });
};

const clean = async () => {
  await fs.rm(path.join(workspaceFolder, BUILD_PATH), { recursive: true, force: true });
  await fs.mkdir(path.join(workspaceFolder, BUILD_PATH));

  const shaderFiles = await glob(path.join(workspaceFolder, 'assets', 'shaders', '*.spv').replace(/\\/g, '/'));
  for (const shaderFile of shaderFiles) {
    await fs.rm(shaderFile, { force: true });
  }
};

const compile_test = async (basename) => {
  console.log(`compiling ${basename}...`);
  const absbuild = (...args) => path.join(workspaceFolder, BUILD_PATH, ...args);

  // compile translation units in parallel (N-at-once)
  const unit_files = [`tests/lib/${basename}.cpp`];
  const dsts = [`tests/lib/${basename}.o`];
  for (const u of COMPILER_TRANSLATION_UNITS) {
    unit_files.push(...await glob(path.relative(workspaceFolder, absbuild(u)).replace(/\\/g, '/')));
    dsts.push(rel(workspaceFolder, BUILD_PATH, 'placeholder', u.replace(RX_EXT, '.o')));
  }
  const compileTranslationUnit = async (unit) => {
    const dir = path.relative(process.cwd(), absbuild(path.dirname(unit)));
    await fs.mkdir(dir, { recursive: true });

    const src = rel(workspaceFolder, unit);
    const dst = rel(workspaceFolder, BUILD_PATH, unit.replace(RX_EXT, '.o'));

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

    const is_c = RX_C.test(src);
    await child_spawn((is_c ? C_COMPILER_PATH : COMPILER_PATH), [
      ...DEBUG_COMPILER_ARGS,
      ...(is_c ? [] : CPP_COMPILER_ARGS),
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
    await child_spawn(`${basename}.exe`);
  }
  console.log("done.");
};

const [, , ...cmds] = process.argv;
loop:
for (const cmd of cmds) {
  switch (cmd) {
    case 'all':
      all();
      break;
    case 'clean':
      await clean();
      break;
    case 'copy_dlls':
      copy_dlls();
      break;
    case 'shaders':
      shaders();
      break;
    case 'protobuf':
      protobuf();
      break;
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
  all
    Clean, rebuild, and launch the default app.
  clean
    Delete all build output.
  copy_dlls
    Copy dynamic libraries to build directory.
  shaders
    Compile SPIRV shaders with GLSLC.
  protobuf
    Compile protobuf .cc code and .bin data files.
  compile_commands
    Generate the .json file needed for clangd for vscode extension.
  Audio_test
    Test SDL audio integration.
  Gamepad_test
    Test SDL gamepad integration.
  Lua_test
    Test Lua sandbox integration.
  Pong_test
    Test everything (game demo).
  Protobuf_test
    Test Google Protobuf data read/write.
  Window_test
    Test SDL window integration.
`);
      break loop;
  }
}
