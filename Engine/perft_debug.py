def read_perft_output(filename):
    moves_dict = {}
    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            if line and ':' in line:  # Skip empty lines and ensure line contains ':'
                move, nodes = line.split(':')
                moves_dict[move.strip()] = int(nodes.strip())
    return moves_dict

def compare_perft(engine1_file, engine2_file):
    # Read both outputs
    engine1_moves = read_perft_output(engine1_file)
    engine2_moves = read_perft_output(engine2_file)
    
    # Find differences
    all_moves = set(engine1_moves.keys()) | set(engine2_moves.keys())
    
    print(f"{'Move':<10} {'Engine1':<12} {'Engine2':<12} {'Difference':<12}")
    print("-" * 46)
    
    total_diff = 0
    for move in sorted(all_moves):
        count1 = engine1_moves.get(move, 0)
        count2 = engine2_moves.get(move, 0)
        diff = count1 - count2
        
        if diff != 0:
            print(f"{move:<10} {count1:<12} {count2:<12} {diff:<12}")
            total_diff += abs(diff)
    
    print("\nSummary:")
    print(f"Total moves in Engine1: {len(engine1_moves)}")
    print(f"Total moves in Engine2: {len(engine2_moves)}")
    print(f"Total node difference: {total_diff}")

if __name__ == "__main__":
    # Usage example
    engine1_file = "your_engine_output.txt"
    engine2_file = "stockfish_output.txt"
    compare_perft(engine1_file, engine2_file)