import subprocess
import sys
import os

# ----------------------------------------------------------------------------
# Azure Key Vault code-signing credentials.
# Fill these in locally. Do NOT commit real secret values to git.
# ----------------------------------------------------------------------------
AZURE_KEY_VAULT_URI    = "https://<vault-name>.vault.azure.net/"
AZURE_CLIENT_ID        = "<application (client) id>"
AZURE_TENANT_ID        = "<directory (tenant) id>"
AZURE_CLIENT_SECRET    = "<client secret value>"
AZURE_CERT_NAME        = "<certificate name in key vault>"

TIMESTAMP_URL = "http://timestamp.globalsign.com/tsa/advanced"
DIGEST_ALGORITHM = "sha256"

PLACEHOLDER_MARKERS = ("<", ">")

def check_credentials():
  values = [AZURE_KEY_VAULT_URI, AZURE_CLIENT_ID, AZURE_TENANT_ID, AZURE_CLIENT_SECRET, AZURE_CERT_NAME]
  for value in values:
    if any(marker in value for marker in PLACEHOLDER_MARKERS):
      print("error: AZURE_* constants at the top of this script are still placeholders. Edit sign_binary.py and fill in real values.")
      sys.exit(1)

def main():
  files = sys.argv[1:]
  if not files:
    print("usage: python sign_binary.py <file1> [file2 ...]")
    sys.exit(1)

  missing = [f for f in files if not os.path.isfile(f)]
  if missing:
    print("error: file(s) not found: " + ", ".join(missing))
    sys.exit(1)

  check_credentials()

  cmd = [
    "AzureSignTool", "sign",
    "-kvu", AZURE_KEY_VAULT_URI,
    "-kvi", AZURE_CLIENT_ID,
    "-kvt", AZURE_TENANT_ID,
    "-kvs", AZURE_CLIENT_SECRET,
    "-kvc", AZURE_CERT_NAME,
    "-tr", TIMESTAMP_URL,
    "-td", DIGEST_ALGORITHM,
    "-fd", DIGEST_ALGORITHM,
    "-v",
  ] + files

  try:
    result = subprocess.run(cmd)
  except FileNotFoundError:
    print("error: AzureSignTool not found on PATH. Run: dotnet tool install --global AzureSignTool")
    sys.exit(1)

  sys.exit(result.returncode)

if __name__ == "__main__":
  main()
