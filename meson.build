project('RacerHam', 'cpp', version: '0.1.0', default_options: ['cpp_std=c++20'])

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

bullet_physics = dependency('bullet')
vulkan_headers = dependency('vulkan-headers')
glfw = dependency('glfw3')
memory_allocator = dependency('vulkan-memory-allocator')
glm = dependency('glm')
entt = dependency('entt')

common_include_directories = ['Jnrlib', 'src']
client_include_directories = [common_include_directories]

jnrlib_srcs = [
  'Jnrlib/FileHelpers.cpp',
]

game_srcs = [
  'src/Application.cpp',
  'src/Gameplay/Entity.cpp',
  'src/Gameplay/World.cpp',
  'src/Gameplay/Camera.cpp',
  'src/Gameplay/Systems/Physics.cpp',
  'src/main.cpp',
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
  dependencies: [glfw, vulkan_headers, memory_allocator, bullet_physics, glm, entt],
  install: true,
  install_dir: bin_directory,
)

# ~~~~ Shaders ~~~~
shaders = [
  'src/Renderer/Vulkan/Shaders/basic.vert',
  'src/Renderer/Vulkan/Shaders/basic.frag',
]

shaders_directory = bin_directory / 'Shaders'

foreach shader_file : shaders

  filename = shader_file.split('/')[-1]
  output_file = filename + '.spv'

  custom_target(
    filename,
    output: output_file,
    input: shader_file,
    command: ['glslangValidator', '-V', '@INPUT@', '-o', '@OUTPUT@'],
    install: true,
    install_dir: shaders_directory,
  )

endforeach

# ~~~~ Utility scripts ~~~~

if buildtype == 'debug'

  if system != 'windows'

    # Bash scripts that are useful for debugging
    reinstall_script = configure_file(
      input: 'scripts/reinstall.sh.in',
      output: 'reinstall.sh',
      configuration: {
        'project_name': 'reinstall_script',
        'build_dir': meson.build_root(),
      },
    )

    install_data(reinstall_script, install_dir: bin_directory, install_mode: 'rwxr-xr-x')
  endif

endif
