python3-pip:
  pkg.installed

configure_opengl_vulkan_development:
  pkg.installed:
    - pkgs:
      - libglfw3-dev
      - libglm-dev
      - libvulkan-dev
      - spirv-tools
      - vulkan-tools
      - vulkan-validationlayers-dev

opengl_extension_loader:
  pip.installed:
    - name: glad
    - require:
      - pkg: python3-pip

