# -*- mode: ruby -*-
# vi: set ft=ruby :
#
# Vagrantfile — Spin up a VM for testing the memory manipulation demo
#
# This is the SAFEST way to test the kernel module.
# The module modifies the syscall table at runtime and can
# crash the system. Always test in a VM first!

Vagrant.configure("2") do |config|
  # --- Ubuntu 24.04 LTS (Noble Numbat) ---
  config.vm.box = "ubuntu/noble64"
  config.vm.box_version = "20240423"

  # --- Provider: VirtualBox (default) / libvirt (KVM) ---
  config.vm.provider "virtualbox" do |vb|
    vb.memory = "1024"
    vb.cpus = 2
    vb.name = "memory-manipulation-demo"
  end

  config.vm.provider "libvirt" do |lv|
    lv.memory = "1024"
    lv.cpus = 2
  end

  # --- Forward SSH agent ---
  config.ssh.forward_agent = true

  # --- Provision: clone & build demo ---
  config.vm.provision "shell", inline: <<-SHELL
    set -e

    echo "=== Installing dependencies ==="
    apt-get update
    apt-get install -y \
        build-essential \
        linux-headers-$(uname -r) \
        gcc make \
        default-jdk default-jre \
        git

    echo "=== Cloning demo ==="
    if [ -d /vagrant ]; then
        cp -r /vagrant /home/vagrant/memory-manipulation-demo
        chown -R vagrant:vagrant /home/vagrant/memory-manipulation-demo
    fi

    cd /home/vagrant/memory-manipulation-demo

    echo "=== Building kernel module ==="
    make -C kernel-module

    echo "=== Building C program ==="
    make -C userspace-c

    echo "=== Building Java program ==="
    cd userspace-java && javac ReadNumber.java && cd ..

    echo ""
    echo "============================================"
    echo "  VM ready! SSH in and test:"
    echo "  vagrant ssh"
    echo "  cd /home/vagrant/memory-manipulation-demo"
    echo "  ./scripts/load_module.sh"
    echo "  echo '42' | ./userspace-c/read_number"
    echo "============================================"
  SHELL

  # --- Sync project folder ---
  config.vm.synced_folder ".", "/vagrant", type: "rsync",
    rsync__exclude: [
      ".git/",
      "kernel-module/*.o",
      "kernel-module/*.ko",
      "kernel-module/*.mod*",
      "kernel-module/Module*",
      "userspace-c/read_number",
      "userspace-java/*.class",
    ]
end
