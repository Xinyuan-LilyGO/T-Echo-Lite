import os
import shutil
from getpass import getuser

print(f"\nCurrent username: {getuser()}\n")

def copy_file_or_folder(source_path, target_path):
    # 获取源文件或文件夹的名称
    source_name = os.path.basename(source_path)
    
    # 构建目标路径
    target_path = os.path.join(target_path, source_name)

    # 检查源路径是否为文件夹
    if os.path.isdir(source_path):
        # 如果是文件夹，则复制文件夹
        if os.path.exists(target_path):
            while True:
                choice = input(f"Target [{os.path.basename(source_path)}] folder already exists: {os.path.dirname(target_path)} \nDo you want to overwrite? [y/n]: ")
                if choice.lower() == 'y':
                    try:
                        # 递归复制文件夹，保留已有文件
                        for item in os.listdir(source_path):
                            s = os.path.join(source_path, item)
                            d = os.path.join(target_path, item)
                            if os.path.isdir(s):
                                shutil.copytree(s, d, dirs_exist_ok=True)
                            else:
                                shutil.copy2(s, d)
                        print(f"Successfully copied [{os.path.basename(source_path)}] folder to: {os.path.dirname(target_path)}")
                    except Exception as e:
                        print(f"Copy failed: {e}")
                        return False
                    break
                elif choice.lower() == 'n':
                    print("Skipping copy")
                    break
                else:
                    print("Invalid input, please enter [y/n]")
        else:
            try:
                # 如果没有此文件夹就创建该文件夹
                os.makedirs(target_path, exist_ok=True)
                # 复制整个文件夹
                shutil.copytree(source_path, target_path, dirs_exist_ok=True)
                print(f"Successfully copied [{os.path.basename(source_path)}] folder to: {os.path.dirname(target_path)}")
            except Exception as e:
                print(f"Copy failed: {e}")
                return False
    else:
        # 如果是文件，则复制文件
        if os.path.exists(target_path):
            while True:
                choice = input(f"Target [{os.path.basename(source_path)}] file already exists: {os.path.dirname(target_path)} \nDo you want to overwrite? [y/n]: ")
                if choice.lower() == 'y':
                    try:
                        shutil.copy2(source_path, target_path)
                        print(f"Successfully copied [{os.path.basename(source_path)}] file to: {os.path.dirname(target_path)}")
                    except Exception as e:
                        print(f"Copy failed: {e}")
                        return False
                    break
                elif choice.lower() == 'n':
                    print("Skipping copy")
                    break
                else:
                    print("Invalid input, please enter [y/n]")
        else:
            try:
                shutil.copy2(source_path, target_path)
                print(f"Successfully copied [{os.path.basename(source_path)}] file to: {os.path.dirname(target_path)}")
            except Exception as e:
                print(f"Copy failed: {e}")
                return False

    return True

def copy_operation(source_paths, target_path):
    # 执行复制操作
    for source_path in source_paths:
        if not os.path.exists(source_path):
            print(f"Source path does not exist: {source_path}")
            return False
        else:
            copy_result = copy_file_or_folder(source_path, target_path)
            if copy_result == False:
                return False

    return True

if __name__ == "__main__":
    # 设置源文件夹路径和目标文件夹路径
    source_path_1 = [
        r"boards\\lilygo_t_echo_lite_nrf52840.json",
    ]
    source_path_2 = [
        r"bootloader\\t_echo_lite_nrf52840",
    ]
    source_path_3 = [
        r"variants\\t_echo_lite_nrf52840",
    ]
    target_path_1 = rf"C:\\Users\\{getuser()}\\.platformio\\platforms\\nordicnrf52\\boards"
    target_path_2 = rf"C:\\Users\\{getuser()}\\.platformio\\packages\\framework-arduinoadafruitnrf52\\bootloader"
    target_path_3 = rf"C:\\Users\\{getuser()}\\.platformio\\packages\\framework-arduinoadafruitnrf52\\variants"

    Installation_judgment = True
    if copy_operation(source_path_1,target_path_1) == False:
        Installation_judgment = False
    if copy_operation(source_path_2,target_path_2) == False:
        Installation_judgment = False
    if copy_operation(source_path_3,target_path_3) == False:
        Installation_judgment = False

    if Installation_judgment == False:
        print("\nt-echo-lite_v1.0.0 board installation failed")
    else:
        print("\nt-echo-lite_v1.0.0 board installation successful")
    



