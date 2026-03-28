import subprocess

for i in range(1, 101):
    ip = f"192.168.1.{i}"
    print(f"\nPinging {ip}...")

    # -n 2 = send 2 echo requests (Windows ping syntax)
    result = subprocess.run(
        ["ping", "-n", "1", ip],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    print(result.stdout)

