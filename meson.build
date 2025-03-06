project('RacerHam', 'cpp', version: '0.1.0', default_options: ['cpp_std=c++23'])

buildtype = get_option('buildtype')

if buildtype == 'debug'
  add_project_arguments('-DDEBUG', language: 'cpp')
else
  add_project_arguments('-DRELEASE', language: 'cpp')
endif

system = host_machine.system()
if system == 'windows'
  add_project_arguments('-DOS_WINDOWS', language: 'cpp')
endif

add_global_arguments('-DVK_NO_PROTOTYPES', language: ['c', 'cpp'])


vulkan_headers = dependency('vulkan-headers')
glfw = dependency('glfw3')
memory_allocator = dependency('vulkan-memory-allocator')


common_include_directories = ['Jnrlib']
client_include_directories = [common_include_directories]

jnrlib_srcs = [
  'Jnrlib/FileHelpers.cpp',
]

game_srcs = [
  'src/main.cpp',
  'src/Application.cpp',
  'src/Renderer/Vulkan/CommandList.cpp',
  'src/Renderer/Vulkan/Image.cpp',
  'src/Renderer/Vulkan/LayoutTracker.cpp',
  'src/Renderer/Vulkan/MemoryAllocator.cpp',
  'src/Renderer/Vulkan/MemoryTracker.cpp',
  'src/Renderer/Vulkan/Pipeline.cpp',
  'src/Renderer/Vulkan/RenderPass.cpp',
  'src/Renderer/Vulkan/Renderer.cpp',
  'src/Renderer/Vulkan/RootSignature.cpp',
  'src/Renderer/Vulkan/SynchronizationObjects.cpp',
  'src/Renderer/Vulkan/VulkanLoader.cpp',
]

jnrlib = static_library('jnrlib', jnrlib_srcs)

bin_directory = meson.source_root() / 'bin'

executable(
  'RacerHam',
  sources: game_srcs,
  include_directories: client_include_directories,
  link_with: jnrlib,
  dependencies: [glfw, memory_allocator],
  install: true,
  install_dir: bin_directory
)


# ~~~~ Utility scripts ~~~~


reinstall_script = configure_file(
    input: 'scripts/reinstall.sh.in',
    output: 'reinstall.sh',
    configuration: {
        'project_name': 'reinstall_script',
        'build_dir': meson.build_root(),
    },
)

install_data(reinstall_script, install_dir: bin_directory, install_mode: 'rwxr-xr-x')


