open Constants
open Constants.K

let do_hash str h =
  let bytes = Cryptokit.hash_string h str in
  let buf = Buffer.create ((String.length bytes) * 2) in
  String.iter (fun c -> Buffer.add_string buf (Printf.sprintf "%02x" (int_of_char c))) bytes;
  [String (Buffer.contents buf)]

let hook_sha2_256 c lbl sort config ff = match c with
  [String str] ->
  let h = Cryptokit.Hash.sha2 256 in
  do_hash str h
| _ -> failwith "sha2_256"

let hook_keccak256 c lbl sort config ff = match c with
  [String str] ->
  let h = Cryptokit.Hash.keccak 256 in
  do_hash str h
| _ -> failwith "keccak256"

let hook_ripemd160 c lbl sort config ff = match c with
  [String str] ->
  let h = Cryptokit.Hash.ripemd160 () in
  do_hash str h
| _ -> failwith "ripemd160"

