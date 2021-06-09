# -*- mode: ruby -*-
# vi: set ft=ruby :


PROVIDER = "virtualbox"
BOX = "bento/ubuntu-20.04"
SCRIPT = "./scripts/linux-bootstrap.sh"

# if on windows host without admin rights, warn & exit
if Vagrant::Util::Platform.windows? then
  def running_in_admin_mode?
    (`reg query HKU\\S-1-5-19 2>&1` =~ /ERROR/).nil?
  end

  unless running_in_admin_mode?
    puts "This vagrant makes use of SymLinks to the host. On Windows, Administrative privileges are required to create symlinks (mklink.exe). Try again from an Administrative command prompt."
    exit 1
  end
end

Vagrant.configure(2) do |config|

    config.vm.provider PROVIDER do |vb|
      vb.customize ["setextradata", :id, "VBoxInternal2/SharedFoldersEnableSymlinksCreate/v-root", "1"]
    end

    # config.vm.synced_folder ".", "/var/www", type: "rsync", rsync__args: ["--verbose", "--archive", "--delete", "-z"]

    config.vm.box = BOX

    config.vm.provision "shell", path: SCRIPT
  end