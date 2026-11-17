/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cjoao-de <cjoao-de@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/10 15:23:14 by jcameira          #+#    #+#             */
/*   Updated: 2025/04/20 19:51:15 by cjoao-de         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minirt.h"

void	get_final_color(t_minirt *s, t_pixel *pixel_color, int iter[3])
{
	t_pixel	temp_color;
	float	pixel_center[3];
	float	ray_direction[3];

	pixel_center[x] = s->scene.vp.pixel00l[x]
		+ ((iter[1] + (random_float() - 0.5))
			* s->scene.vp.deltah[x])
		+ ((iter[0] + (random_float() - 0.5)) * s->scene.vp.deltav[x]);
	pixel_center[y] = s->scene.vp.pixel00l[y]
		+ ((iter[1] + (random_float() - 0.5))
			* s->scene.vp.deltah[y])
		+ ((iter[0] + (random_float() - 0.5)) * s->scene.vp.deltav[y]);
	pixel_center[z] = s->scene.vp.pixel00l[z]
		+ ((iter[1] + (random_float() - 0.5))
			* s->scene.vp.deltah[z])
		+ ((iter[0] + (random_float() - 0.5)) * s->scene.vp.deltav[z]);
	vec3_subf(ray_direction, pixel_center, s->scene.cam.o);
	temp_color = ray_color(&s->scene,
			get_ray(s->scene.cam.o, ray_direction), 50);
	*pixel_color = add_pixel_color(*pixel_color, temp_color);
}

void	progressive_accumulate_pixel(t_minirt *s, int y, int x)
{
	t_pixel	temp_color;
	float	pixel_center[3];
	float	ray_direction[3];
	int		idx;

	pixel_center[0] = s->scene.vp.pixel00l[0]
		+ ((x + (random_float() - 0.5)) * s->scene.vp.deltah[0])
		+ ((y + (random_float() - 0.5)) * s->scene.vp.deltav[0]);
	pixel_center[1] = s->scene.vp.pixel00l[1]
		+ ((x + (random_float() - 0.5)) * s->scene.vp.deltah[1])
		+ ((y + (random_float() - 0.5)) * s->scene.vp.deltav[1]);
	pixel_center[2] = s->scene.vp.pixel00l[2]
		+ ((x + (random_float() - 0.5)) * s->scene.vp.deltah[2])
		+ ((y + (random_float() - 0.5)) * s->scene.vp.deltav[2]);
	vec3_subf(ray_direction, pixel_center, s->scene.cam.o);
	temp_color = ray_color(&s->scene,
			get_ray(s->scene.cam.o, ray_direction), 50);
	idx = (y * W + x) * 3;
	s->scene.cam.progressive_accumulator[idx] += temp_color.r;
	s->scene.cam.progressive_accumulator[idx + 1] += temp_color.g;
	s->scene.cam.progressive_accumulator[idx + 2] += temp_color.b;
}

void	progressive_update_display(t_minirt *s)
{
	int		y;
	int		x;
	int		idx;
	t_pixel	averaged_color;
	float	pass_count;

	pass_count = (float)s->scene.cam.progressive_pass_count;
	y = -1;
	while (++y < H)
	{
		x = -1;
		while (++x < W)
		{
			idx = (y * W + x) * 3;
			averaged_color.r = (int)(s->scene.cam.progressive_accumulator[idx]
					/ pass_count);
			averaged_color.g = (int)(s->scene.cam.progressive_accumulator[idx
					+ 1] / pass_count);
			averaged_color.b = (int)(s->scene.cam.progressive_accumulator[idx
					+ 2] / pass_count);
			gamma_correction(&averaged_color);
			pixel_put(&s->scene.cam.img, x, y, averaged_color.rgb);
		}
	}
}

int	render_progressive(t_minirt *s)
{
	int		y;
	int		x;
	time_t	time;

	if (s->scene.cam.progressive_pass_count >= s->scene.cam.progressive_target_passes
		|| s->scene.cam.progressive_pass_count >= PROGRESSIVE_MAX_PASSES)
		return (0);
	time = get_time();
	s->scene.cam.progressive_pass_count++;
	y = -1;
	while (++y < H)
	{
		x = -1;
		while (++x < W)
			progressive_accumulate_pixel(s, y, x);
	}
	progressive_update_display(s);
	mlx_put_image_to_window(s->mlx, s->win_rayt, s->scene.cam.img.image, 0, 0);
	ft_dprintf(1, "Progressive render pass %d/%d (max: %d)\r\n",
		s->scene.cam.progressive_pass_count,
		s->scene.cam.progressive_target_passes, PROGRESSIVE_MAX_PASSES);
	return (print_time(get_time() - time), 0);
}

void	reset_progressive_render(t_minirt *s)
{
	int	size;

	size = W * H * 3 * sizeof(float);
	ft_bzero(s->scene.cam.progressive_accumulator, size);
	s->scene.cam.progressive_pass_count = 0;
	s->scene.cam.progressive_target_passes = PROGRESSIVE_INCREMENT;
}

int	render_rayt(t_minirt *s)
{
	t_pixel	pixel_color;
	int		iter[3];
	time_t	time;

	time = get_time();
	iter[0] = -1;
	while (++iter[0] < H)
	{
		iter[1] = -1;
		while (++iter[1] < W)
		{
			ft_bzero(&pixel_color, sizeof(pixel_color));
			iter[2] = -1;
			while (++iter[2] < (int)(RAYS_PER_PIXEL * s->scene.quality))
				get_final_color(s, &pixel_color, iter);
			anti_aliasing_get_color(&pixel_color,
				RAYS_PER_PIXEL * s->scene.quality);
			gamma_correction(&pixel_color);
			pixel_put(&s->scene.cam.img, iter[1], iter[0], pixel_color.rgb);
		}
	}
	mlx_put_image_to_window(s->mlx, s->win_rayt, s->scene.cam.img.image, 0, 0);
	return (print_clean(s), print_time(get_time() - time), 0);
}

int	minirt(t_minirt *s)
{
	static int	loop_counter;

	if (loop_counter == 0)
		mlx_string_put(s->mlx, s->win_rayt, H / 2, W / 2, WHITE, \
			"M O V E   T H I S   W I N D O W");
	if (s->render)
	{
		if (s->scene.cam.progressive_enabled)
		{
			// ft_dprintf(1, "Raytracer running pass: %i\r", loop_counter);
			render_progressive(s);
			loop_counter = 1;
		}
		else
		{
			ft_dprintf(1, "Raytracer running loop: %i\r", loop_counter);
			render_rayt(s);
			dup_image(s->scene.cam.copy, s->scene.cam.img.data);
			loop_counter++;
		}
	}
	return (0);
}

void	setup_minirt(char **argv)
{
	static t_minirt	s;
	t_scene			scene;

	ft_bzero((void *)&s, sizeof(t_minirt));
	ft_bzero((void *)&scene, sizeof(scene));
	s.scene = scene;
	s.mlx = mlx_init();
	if (!s.mlx)
		end_minirt(&s);
	if (!parser(&s.scene, argv[1])
		|| !check_needed_elements(s.scene, argv[1]))
		end_minirt(&s);
	calc_viewport_info(&s.scene);
	if (!(setup_menu(&s) && setup_assets(&s) && setup_rayt(&s)))
		end_minirt(&s);
	s.render = false;
	s.scene.quality = 1.0;
	s.scene.quality_idx = 1;
	setup_hooks(&s);
	mlx_loop_hook(s.mlx, minirt, &s);
	render_menu(&s);
	mlx_loop(s.mlx);
}

int	main(int argc, char **argv)
{
	if (argc != 2)
		return (ft_dprintf(2, NO_ARGS), 1);
	if (!ft_strnstr(argv[1], ".rt", ft_strlen(argv[1]))
		|| *(ft_strnstr(argv[1], ".rt", ft_strlen(argv[1])) + 3))
		return (ft_dprintf(2, INVALID_RT), 1);
	setup_minirt(argv);
	return (0);
}
